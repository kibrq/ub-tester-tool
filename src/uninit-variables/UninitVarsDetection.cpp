#include "uninit-variables/UninitVarsDetection.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include <iostream>
#include <stdexcept>

using namespace clang;
using namespace llvm;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

namespace ub_tester {

FindFundTypeVarDeclVisitor::FindFundTypeVarDeclVisitor(ASTContext* Context) : Context_(Context) {}

FindSafeTypeAccessesVisitor::FindSafeTypeAccessesVisitor(ASTContext* Context) : Context_(Context) {}

FindSafeTypeOperatorsVisitor::FindSafeTypeOperatorsVisitor(ASTContext* Context) : Context_(Context) {}

bool FindFundTypeVarDeclVisitor::VisitVarDecl(VarDecl* VDecl) {
  if (!Context_->getSourceManager().isWrittenInMainFile(VDecl->getBeginLoc()))
    return true;

  QualType VariableType = VDecl->getType().getUnqualifiedType();
  if (VariableType.getNonReferenceType()->isFundamentalType() && !(VDecl->isLocalVarDeclOrParm() && !(VDecl->isLocalVarDecl()))) {
    if (VDecl->hasInit()) {
      Expr* InitExpr = dyn_cast_or_null<Expr>(*(VDecl->getInitAddress()));
      assert(InitExpr);

      SubstitutionASTWrapper(Context_)
          .setLoc(getAfterNameLoc(VDecl, Context_))
          .setPrior(SubstPriorityKind::Deep)
          .setFormats("#@", "(@)")
          .setArguments(InitExpr)
          .apply();
    }
  }
  return true;
}

namespace {

bool isDeclRefExprToLocalVarOrParmOrMember(DeclRefExpr* DRExpr) {
  if (!DRExpr)
    return false;
  VarDecl* VDecl = dyn_cast_or_null<VarDecl>(DRExpr->getDecl());
  MemberExpr* MembExpr = dyn_cast_or_null<MemberExpr>(DRExpr->getExprStmt());
  return (VDecl && VDecl->isLocalVarDeclOrParm()) || (MembExpr);
}

} // namespace

bool FindSafeTypeAccessesVisitor::VisitDeclRefExpr(DeclRefExpr* DRExpr) {
  if (!Context_->getSourceManager().isWrittenInMainFile(DRExpr->getBeginLoc()))
    return true;

  // check if there is a suitable MemberExpr
  // just a single step up or processing time gets extremely long
  DynTypedNode DRExprParentIterNode = DynTypedNode::create<>(*DRExpr);
  const DynTypedNodeList DRExprParentNodeList = ParentMapContext(*Context_).getParents(DRExprParentIterNode);
  DRExprParentIterNode = DRExprParentNodeList[0];
  const MemberExpr* MembExpr = DRExprParentIterNode.get<MemberExpr>();
  bool FoundCorrespMembExpr = false;
  if (MembExpr && dyn_cast<DeclRefExpr>(MembExpr->getBase()) == DRExpr)
    FoundCorrespMembExpr = true;

  QualType VarType = FoundCorrespMembExpr ? MembExpr->getType() : DRExpr->getDecl()->getType();
  if (!(VarType.getNonReferenceType()->isFundamentalType() && isDeclRefExprToLocalVarOrParmOrMember(DRExpr)))
    return true;
  std::string VarName = DRExpr->getNameInfo().getName().getAsString();
  if (FoundCorrespMembExpr)
    VarName += ("." + MembExpr->getMemberNameInfo().getAsString()); // does not support nested classes' members
  // check for value access
  bool FoundCorrespImplicitCast = false;

  for (DRExprParentIterNode = DynTypedNode::create<>(*DRExpr); !FoundCorrespImplicitCast;) {
    const DynTypedNodeList DRExprParentNodeList = ParentMapContext(*Context_).getParents(DRExprParentIterNode);
    if (DRExprParentNodeList.empty())
      break;
    DRExprParentIterNode = DRExprParentNodeList[0];
    const ImplicitCastExpr* ImplicitCast = DRExprParentIterNode.get<ImplicitCastExpr>();
    // backwards check DISABLED due to CompAssignOp
    if (ImplicitCast && ImplicitCast->getCastKind() == CastKind::CK_LValueToRValue &&
        ImplicitCast->getSubExpr()->getType().getNonReferenceType()->isFundamentalType()) {
      FoundCorrespImplicitCast = true;
      SubstitutionASTWrapper(Context_)
          .setLoc(DRExpr->getBeginLoc())
          .setPrior(SubstPriorityKind::Deep)
          .setFormats("#@", "ASSERT_GET_VALUE(@)")
          .setArguments(VarName)
          .apply();
    }
  }

  if (FoundCorrespImplicitCast)
    return true;

  // then reference access
  bool FoundCallingFunction = false;
  bool FoundCodeAvailCallingFunction = false;
  for (DRExprParentIterNode = DynTypedNode::create<>(*DRExpr); !FoundCallingFunction;) {
    const DynTypedNodeList ParentNodeList = ParentMapContext(*Context_).getParents(DRExprParentIterNode);
    if (ParentNodeList.empty())
      break;
    DRExprParentIterNode = ParentNodeList[0];
    const CallExpr* CallingFunction = DRExprParentIterNode.get<CallExpr>();
    // TODO: backwards check
    if (CallingFunction) {
      FoundCallingFunction = true;
      if (func_code_avail::hasFuncAvailCode(CallingFunction->getDirectCallee()))
        FoundCodeAvailCallingFunction = true;
    }
  }
  if (FoundCodeAvailCallingFunction || !FoundCallingFunction)
    return true;
  else
    // set ignore for functions with inaccessible code
    SubstitutionASTWrapper(Context_)
        .setLoc(DRExpr->getBeginLoc())
        .setPrior(SubstPriorityKind::Deep)
        .setFormats("#@", "ASSERT_GET_REF_IGNORE(@)")
        .setArguments(VarName)
        .apply();
  if (!FoundCallingFunction)
    SubstitutionASTWrapper(Context_)
        .setLoc(DRExpr->getBeginLoc())
        .setPrior(SubstPriorityKind::Deep)
        .setFormats("#@", "ASSERT_GET_REF_IGNORE(@)")
        .setArguments(VarName)
        .apply();
  return true;
}

bool FindSafeTypeOperatorsVisitor::VisitBinaryOperator(BinaryOperator* Binop) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Binop->getBeginLoc()))
    return true;

  QualType BinopLHSType = Binop->getLHS()->getType();
  if (!(Binop->isAssignmentOp() && BinopLHSType->isFundamentalType() &&
        isDeclRefExprToLocalVarOrParmOrMember(dyn_cast_or_null<DeclRefExpr>(Binop->getLHS()))))
    return true;
  if (!Binop->isCompoundAssignmentOp())
    SubstitutionASTWrapper(Context_)
        .setLoc(Binop->getBeginLoc())
        .setPrior(SubstPriorityKind::Deep)
        .setFormats("@#@", "ASSERT_SET_VALUE(@, @)")
        .setArguments(Binop->getLHS(), Binop->getRHS())
        .apply();
  else
    SubstitutionASTWrapper(Context_)
        .setLoc(Binop->getBeginLoc())
        .setPrior(SubstPriorityKind::Deep)
        .setFormats("@", "ASSERT_GET_REF(@)")
        .setArguments(Binop->getLHS())
        .apply();
  return true;
}

bool FindSafeTypeOperatorsVisitor::VisitUnaryOperator(UnaryOperator* Unop) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Unop->getBeginLoc()))
    return true;

  QualType UnopExprType = Unop->getSubExpr()->getType();
  if (!(Unop->getSubExpr()->getType()->isFundamentalType() && (Unop->isIncrementDecrementOp())))
    return true;
  // else there will be LRValue conversion, other cases
  SubstitutionASTWrapper(Context_)
      .setLoc(Unop->getSubExpr()->getBeginLoc())
      .setPrior(SubstPriorityKind::Deep)
      .setFormats("$@", "ASSERT_GET_REF(@)")
      .setArguments(Unop->getSubExpr())
      .apply();
  return true;
}

FindUninitVarsConsumer::FindUninitVarsConsumer(ASTContext* Context)
    : FundamentalTypeVarDeclVisitor_(Context), SafeTypeAccessesVisitor_(Context), SafeTypeOperatorsVisitor_(Context) {}

void FindUninitVarsConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FundamentalTypeVarDeclVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeOperatorsVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeAccessesVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
