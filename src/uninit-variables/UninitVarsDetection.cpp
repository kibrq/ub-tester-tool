#include "uninit-variables/UninitVarsDetection.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

// #include "clang/AST/ParentMap.h"
#include "clang/AST/ParentMapContext.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#include <iostream>
#include <stdexcept>

namespace ub_tester {

// all constructors

FindFundTypeVarDeclVisitor::FindFundTypeVarDeclVisitor(ASTContext* Context)
    : Context(Context) {}
FindSafeTypeAccessesVisitor::FindSafeTypeAccessesVisitor(ASTContext* Context)
    : Context(Context) {}
FindSafeTypeDefinitionsVisitor::FindSafeTypeDefinitionsVisitor(
    ASTContext* Context)
    : Context(Context) {}

// substitute types (i.e. 'int' -> 'safe_T<int>')
// TODO: preserve 'static', 'const' (?) and other (?) keywords
bool FindFundTypeVarDeclVisitor::VisitVarDecl(VarDecl* VariableDecl) {
  if (!Context->getSourceManager().isInMainFile(VariableDecl->getBeginLoc()))
    return true;

  clang::QualType VariableType = VariableDecl->getType().getUnqualifiedType();
  if (VariableType.getTypePtr()->isFundamentalType() &&
      !(VariableDecl->isLocalVarDeclOrParm() &&
        !VariableDecl->isLocalVarDecl())) {

    if (VariableDecl->hasInit()) {

      Expr* InitializationExpr =
          dyn_cast_or_null<Expr>(*(VariableDecl->getInitAddress()));
      assert(InitializationExpr);

      // std::cout << getExprAsString(InitializationExpr, Context) << std::endl;
      // TODO: maybe shift here by 1?

      SubstitutionASTWrapper(Context)
          .setLoc(getAfterNameLoc(VariableDecl, Context))
          .setPrior(SubstPriorityKind::Deep)
          .setFormats("#@", "{@}")
          .setArguments(InitializationExpr)
          .apply();
    }
  }
  return true;
}

// TODO: move to corresponding class
// bool IsFunctionUndetectable(CallExpr* FuncCallExpr) {
//   FunctionDecl* FuncDecl = FuncCallExpr->getDirectCallee();
//   if (FuncDecl == nullptr || !FuncDecl->isDefined()) {
//     std::cout << "got undetectable function" << std::endl;
//     return true;
//   }
//   return false;
// }

// detect variable usage; substutute with get OR getIgnore
bool FindSafeTypeAccessesVisitor::VisitImplicitCastExpr(ImplicitCastExpr* ICE) {
  if (!Context->getSourceManager().isInMainFile(ICE->getBeginLoc()))
    return true;

  if (ICE->getCastKind() == CastKind::CK_LValueToRValue &&
      ICE->getType().getTypePtr()->isFundamentalType()) {
    // assuming all fundamental types are already Safe_T

    const DeclRefExpr* UnderlyingDRE = dyn_cast<DeclRefExpr>(ICE->getSubExpr());
    if (UnderlyingDRE != nullptr) {
      // LocationRange UnderlyingDRERange =
      // GetLocationRange(UnderlyingDRE->getSourceRange(),
      // Context->getSourceManager());

      // TODO: ensure unimportance of following line
      // UnderlyingDRERange.second.second += 1;

      std::string UnderlyingVarName =
          UnderlyingDRE->getNameInfo().getName().getAsString();

      SubstitutionASTWrapper(Context)
          .setLoc(UnderlyingDRE->getBeginLoc())
          .setPrior(SubstPriorityKind::Deep)
          .setFormats("#@",
                      "@." + UB_UninitSafeTypeConsts::GETMETHOD_NAME + "()")
          .setArguments(UnderlyingVarName)
          .apply();
    }

    // ! problems with Parent-related documentation
    // ? check if a node is an argument in function call ?
    // ? then in 'bad' function call ?

    bool FoundCallingFunction = false;
    DynTypedNode ParentIterNode = DynTypedNode::create<>(*ICE);
    do {
      const DynTypedNodeList ParentNodeList =
          ParentMapContext(*Context).getParents(ParentIterNode);
      if (ParentNodeList.empty())
        break;

      ParentIterNode = ParentNodeList[0];
      const CallExpr* CallingFunction = ParentIterNode.get<CallExpr>();

      if (CallingFunction) {
        // THIS FINALLY WORKS
        FoundCallingFunction = true;
      }

    } while (!FoundCallingFunction);
  }

  return true;
}

// detect Safe_T assignments; substitute with .init() function
bool FindSafeTypeDefinitionsVisitor::VisitBinaryOperator(
    BinaryOperator* BinOp) {
  if (!Context->getSourceManager().isInMainFile(BinOp->getBeginLoc()))
    return true;

  if (BinOp->isAssignmentOp() &&
      BinOp->getLHS()->getType().getTypePtr()->isFundamentalType()) {
    // assuming all fundamental types are already Safe_T
    // TODO: replace 'assuming' with assert (?)

    // LocationRange DefinitionExprLocationRange =
    // GetLocationRange(BinOp->getRHS()->getSourceRange(),
    // Context->getSourceManager()); std::string substitution = '.' +
    // UB_UninitSafeTypeConsts::INITMETHOD_NAME + "( [[ file contents from " +
    //                            LocPairToString(DefinitionExprLocationRange.first)
    //                            + " to " +
    //                            LocPairToString(DefinitionExprLocationRange.second)
    //                            + " ]] )";

    SubstitutionASTWrapper(Context)
        .setLoc(BinOp->getLHS()->getEndLoc())
        .setPrior(SubstPriorityKind::Deep)
        .setFormats("@#@",
                    "@." + UB_UninitSafeTypeConsts::INITMETHOD_NAME + "(@)")
        .setArguments(BinOp->getLHS(), BinOp->getRHS())
        .apply();
  }

  return true;
}

// Consumer implementation

AssertUninitVarsConsumer::AssertUninitVarsConsumer(ASTContext* Context)
    : FundamentalTypeVarDeclVisitor(Context), SafeTypeAccessesVisitor(Context),
      SafeTypeDefinitionsVisitor(Context) {}

void AssertUninitVarsConsumer::HandleTranslationUnit(
    clang::ASTContext& Context) {
  FundamentalTypeVarDeclVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeDefinitionsVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  SafeTypeAccessesVisitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
