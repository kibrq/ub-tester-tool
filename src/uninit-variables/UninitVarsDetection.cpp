#include "uninit-variables/UninitVarsDetection.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "uninit-variables/UB_UninitSafeType.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/AST/ParentMapContext.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#include <UBUtility.h>
#include <iostream>
#include <stdexcept>

namespace ub_tester {

// all constructors

FindFundTypeVarDeclVisitor::FindFundTypeVarDeclVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeAccessesVisitor::FindSafeTypeAccessesVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeDefinitionsVisitor::FindSafeTypeDefinitionsVisitor(ASTContext* Context) : Context(Context) {}

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

// detect variable usage; substutute with get OR getIgnore
bool FindSafeTypeAccessesVisitor::VisitDeclRefExpr(DeclRefExpr* DRE) {
  if (!Context->getSourceManager().isInMainFile(DRE->getBeginLoc()))
    return true;

  if (DRE->getDecl()->getType().getNonReferenceType()->isFundamentalType()) {

    std::string VarName = DRE->getNameInfo().getName().getAsString();

    // check if value access
    bool FoundCorrespICE = false;
    DynTypedNode DREParentIterNode = DynTypedNode::create<>(*DRE);
    do {
      const DynTypedNodeList DREParentNodeList = ParentMapContext(*Context).getParents(DREParentIterNode);
      if (DREParentNodeList.empty())
        break;

      DREParentIterNode = DREParentNodeList[0];
      const ImplicitCastExpr* ICE = DREParentIterNode.get<ImplicitCastExpr>();

      if (ICE && ICE->getCastKind() == CastKind::CK_LValueToRValue /*&&
          ICE->getType().getTypePtr()->isFundamentalType()*/
          && dyn_cast<DeclRefExpr>(ICE->getSubExpr()) == DRE) {
        FoundCorrespICE = true;

        ASTFrontendInjector::getInstance().substitute(Context, DRE->getBeginLoc(), "#@",
                                                      "@." + UB_UninitSafeTypeConsts::GETMETHOD_NAME + "()", VarName);
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
bool FindSafeTypeDefinitionsVisitor::VisitBinaryOperator(BinaryOperator* BinOp) {
  if (!Context->getSourceManager().isInMainFile(BinOp->getBeginLoc()))
    return true;

  if (BinOp->isAssignmentOp() && BinOp->getLHS()->getType().getTypePtr()->isFundamentalType()) {
    // assuming all fundamental types are already Safe_T
    // TODO: replace 'assuming' with assert (?)

    ASTFrontendInjector::getInstance().substitute(Context, BinOp->getBeginLoc(), "@#=#@",
                                                  "@." + UB_UninitSafeTypeConsts::INITMETHOD_NAME + "(@)", BinOp->getLHS(),
                                                  BinOp->getRHS());
  }

  return true;
}

// Consumer implementation

AssertUninitVarsConsumer::AssertUninitVarsConsumer(ASTContext* Context)
    : FundamentalTypeVarDeclVisitor(Context), SafeTypeAccessesVisitor(Context), SafeTypeDefinitionsVisitor(Context) {}

void AssertUninitVarsConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FundamentalTypeVarDeclVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeDefinitionsVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeAccessesVisitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
