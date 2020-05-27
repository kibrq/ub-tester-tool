#include "uninit-variables/UninitVarsDetection.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "uninit-variables/UB_UninitSafeTypeConsts.h"

#include "clang/AST/ParentMapContext.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;
using namespace llvm;

#include <iostream>
#include <stdexcept>

namespace ub_tester {

// all constructors

FindFundTypeVarDeclVisitor::FindFundTypeVarDeclVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeAccessesVisitor::FindSafeTypeAccessesVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeOperatorsVisitor::FindSafeTypeOperatorsVisitor(ASTContext* Context) : Context(Context) {}

// substitute types (i.e. 'int' -> 'safe_T<int>')
bool FindFundTypeVarDeclVisitor::VisitVarDecl(VarDecl* VariableDecl) {
  if (!Context->getSourceManager().isInMainFile(VariableDecl->getBeginLoc()))
    return true;

  clang::QualType VariableType = VariableDecl->getType().getUnqualifiedType();
  if (VariableType.getNonReferenceType().getTypePtr()->isFundamentalType() &&
      !(VariableDecl->isLocalVarDeclOrParm() && !VariableDecl->isLocalVarDecl())) {

    if (VariableDecl->hasInit()) {

      Expr* InitializationExpr = dyn_cast_or_null<Expr>(*(VariableDecl->getInitAddress()));
      assert(InitializationExpr);

      ASTFrontendInjector::getInstance().substitute(Context, getAfterNameLoc(VariableDecl, Context), "#@", "(@)",
                                                    InitializationExpr);
    }
  }
  return true;
}

bool isDREToLocalVarOrParmOrMember(DeclRefExpr* DRE) {
  if (!DRE)
    return false;
  VarDecl* VD = dyn_cast_or_null<VarDecl>(DRE->getDecl());
  MemberExpr* ME = dyn_cast_or_null<MemberExpr>(DRE->getExprStmt());
  return (VD && VD->isLocalVarDeclOrParm()) || (ME);
}

// detect variable usage; substutute with get OR getIgnore
bool FindSafeTypeAccessesVisitor::VisitDeclRefExpr(DeclRefExpr* DRE) {
  if (!Context->getSourceManager().isWrittenInMainFile(DRE->getBeginLoc()))
    return true;

  QualType VarType;

  // check if thereis a suitable MemberExpr
  // just a single step up or processing time gets extremely long
  bool FoundCorrespME = false;
  DynTypedNode DREParentIterNode = DynTypedNode::create<>(*DRE);
  const MemberExpr* ME = nullptr;
  const DynTypedNodeList DREParentNodeList = ParentMapContext(*Context).getParents(DREParentIterNode);
  DREParentIterNode = DREParentNodeList[0];
  ME = DREParentIterNode.get<MemberExpr>();
  if (ME && dyn_cast<DeclRefExpr>(ME->getBase()) == DRE)
    FoundCorrespME = true;

  if (FoundCorrespME)
    VarType = ME->getType();
  else
    VarType = DRE->getDecl()->getType();

  if (VarType.getNonReferenceType()->isFundamentalType() && isDREToLocalVarOrParmOrMember(DRE)) {

    std::string VarName = DRE->getNameInfo().getName().getAsString();
    if (FoundCorrespME)
      VarName += ("." + ME->getMemberNameInfo().getAsString()); // does not support nested classes' members

    // check if value access
    bool FoundCorrespICE = false;
    DynTypedNode DREParentIterNode = DynTypedNode::create<>(*DRE);
    do {
      const DynTypedNodeList DREParentNodeList = ParentMapContext(*Context).getParents(DREParentIterNode);
      if (DREParentNodeList.empty())
        break;

      DREParentIterNode = DREParentNodeList[0];
      const ImplicitCastExpr* ICE = DREParentIterNode.get<ImplicitCastExpr>();

      if (ICE &&
          ICE->getCastKind() == CastKind::CK_LValueToRValue
          // ! && dyn_cast<DeclRefExpr>(ICE->getSubExpr()) == DRE) {    // backwards check DISABLED due to CAO
          && ICE->getSubExpr()->getType().getNonReferenceType()->isFundamentalType()) {
        FoundCorrespICE = true;

        ASTFrontendInjector::getInstance().substitute(Context, DRE->getBeginLoc(), "#@",
                                                      "@." + UB_UninitSafeTypeConsts::GETMETHOD_NAME +
                                                          "({__FILE__, __LINE__, \"" + VarName + "\", \"" +
                                                          VarType.getAsString() + "\"})",
                                                      VarName);
      }
    } while (!FoundCorrespICE);
    if (FoundCorrespICE)
      return true;

    // then reference access

    bool FoundCallingFunction = false;
    bool FoundCodeAvailCallingFunction = false;
    DynTypedNode ParentIterNode = DynTypedNode::create<>(*DRE);
    do {
      const DynTypedNodeList ParentNodeList = ParentMapContext(*Context).getParents(ParentIterNode);
      if (ParentNodeList.empty())
        break;

      ParentIterNode = ParentNodeList[0];
      const CallExpr* CallingFunction = ParentIterNode.get<CallExpr>();

      // TODO: backwards check

      if (CallingFunction) {
        FoundCallingFunction = true;

        if (func_code_avail::hasFuncAvailCode(CallingFunction->getDirectCallee())) {
          // return entire object - goes by reference
          // equals 'do nothing with code'
          FoundCodeAvailCallingFunction = true;
        }
      }

    } while (!FoundCallingFunction);
    if (FoundCodeAvailCallingFunction || !FoundCallingFunction) {
      // 'good' function
      // do nothing!
      return true;
    } else {
      // set ignore for 'bad' functions
      ASTFrontendInjector::getInstance().substitute(Context, DRE->getBeginLoc(), "#@",
                                                    "@." + UB_UninitSafeTypeConsts::GETIGNOREMETHOD_NAME + "()", VarName);
      // TODO: send warning
    }

    if (!FoundCallingFunction) {
      // as for ref v1, giving out all references is ignored
      // ? for ref v2: maybe do nothing, since such reference can only be used to init another?
      ASTFrontendInjector::getInstance().substitute(Context, DRE->getBeginLoc(), "#@",
                                                    "@." + UB_UninitSafeTypeConsts::GETIGNOREMETHOD_NAME + "()", VarName);
    }
  } // else not even fundamental type

  return true;
}

// detect Safe_T assignments; substitute with .init() function
bool FindSafeTypeOperatorsVisitor::VisitBinaryOperator(BinaryOperator* BinOp) {
  if (!Context->getSourceManager().isInMainFile(BinOp->getBeginLoc()))
    return true;

  if (BinOp->isAssignmentOp() && BinOp->getLHS()->getType().getTypePtr()->isFundamentalType() &&
      isDREToLocalVarOrParmOrMember(dyn_cast_or_null<DeclRefExpr>(BinOp->getLHS()))) {

    if (!BinOp->isCompoundAssignmentOp()) {
      ASTFrontendInjector::getInstance().substitute(Context, BinOp->getBeginLoc(), "@#=#@",
                                                    "@." + UB_UninitSafeTypeConsts::INITMETHOD_NAME + "(@)", BinOp->getLHS(),
                                                    BinOp->getRHS());
    } else {
      // CAOs do not cause LRValue conversion, but still require the value
      ASTFrontendInjector::getInstance().substitute(Context, BinOp->getBeginLoc(), "@",
                                                    "@." + UB_UninitSafeTypeConsts::GETREFMETHOD_NAME + "()", BinOp->getLHS());
    }
  }

  return true;
}

bool FindSafeTypeOperatorsVisitor::VisitUnaryOperator(UnaryOperator* UnOp) {
  if (!Context->getSourceManager().isInMainFile(UnOp->getBeginLoc()))
    return true;
  if (UnOp->getSubExpr()->getType()->isFundamentalType() && (UnOp->isIncrementDecrementOp())) {
    ASTFrontendInjector::getInstance().substitute(Context, UnOp->getBeginLoc(), "#@",
                                                  "@." + UB_UninitSafeTypeConsts::GETREFMETHOD_NAME + "()", UnOp->getSubExpr());
  } // else there will be LRValue conversion, another case
  return true;
}

// Consumer implementation

AssertUninitVarsConsumer::AssertUninitVarsConsumer(ASTContext* Context)
    : FundamentalTypeVarDeclVisitor(Context), SafeTypeAccessesVisitor(Context), SafeTypeOperatorsVisitor(Context) {}

void AssertUninitVarsConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FundamentalTypeVarDeclVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeOperatorsVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeAccessesVisitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
