#include "uninit-variables/UninitVarsDetection.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#include <UBUtility.h>
#include <iostream>
#include <stdexcept>

namespace ub_tester {

namespace UB_UninitSafeTypeConsts {
const std::string TEMPLATE_NAME = "UB_UninitSafeType";
const std::string GETMETHOD_NAME = "getValue";
const std::string INITMETHOD_NAME = "tryInitValue";
} // namespace UB_UninitSafeTypeConsts

using LocationPair = std::pair<unsigned int, unsigned int>;

LocationPair GetLocationPair(SourceLocation&& Location, const SourceManager& Manager) {
  return LocationPair(Manager.getSpellingLineNumber(Location), Manager.getSpellingColumnNumber(Location));
}

std::string LocPairToString(const LocationPair& LP) {
  std::stringstream res{};
  res << LP.first << ':' << LP.second;
  return res.str();
}

using LocationRange = std::pair<LocationPair, LocationPair>;

LocationRange GetLocationRange(const SourceRange& SR, const SourceManager& Manager) {
  return LocationRange(GetLocationPair(SR.getBegin(), Manager), GetLocationPair(SR.getEnd(), Manager));
}

std::pair<std::string, std::string> LocRangeToStrings(const SourceRange&& SR, const SourceManager& Manager) {
  LocationRange LR = GetLocationRange(SR, Manager);
  return std::pair(LocPairToString(LR.first), LocPairToString(LR.second));
}

// all constructors

FindFundTypeVarDeclVisitor::FindFundTypeVarDeclVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeAccessesVisitor::FindSafeTypeAccessesVisitor(ASTContext* Context) : Context(Context) {}
FindSafeTypeDefinitionsVisitor::FindSafeTypeDefinitionsVisitor(ASTContext* Context) : Context(Context) {}

// substitute types (i.e. 'int' -> 'safe_T<int>')
bool FindFundTypeVarDeclVisitor::VisitVarDecl(VarDecl* VariableDecl) {
  if (!Context->getSourceManager().isInMainFile(VariableDecl->getBeginLoc()))
    return true;

  clang::QualType VariableType = VariableDecl->getType().getUnqualifiedType();
  if (VariableType.getTypePtr()->isFundamentalType()) {
    std::string TypeAndNameSubstitution =
        UB_UninitSafeTypeConsts::TEMPLATE_NAME + "<" + VariableType.getAsString() + "> " + VariableDecl->getNameAsString();
    std::pair DeclRangeEndsStr = LocRangeToStrings(VariableDecl->getSourceRange(), Context->getSourceManager());

    if (VariableDecl->hasInit()) {
      assert(VariableDecl->getInitAddress());
      Stmt* InitializationStmt = *(VariableDecl->getInitAddress());
      std::pair InitializationRangeEndsStr = LocRangeToStrings(InitializationStmt->getSourceRange(), Context->getSourceManager());
      std::string InitSubstitution = TypeAndNameSubstitution + "( [[ file contents from " + InitializationRangeEndsStr.first + " to " +
                                     InitializationRangeEndsStr.second + " ]], __LINE__)";
      std::cout << "replace: " << InitSubstitution << "\t from " << DeclRangeEndsStr.first << " to " << DeclRangeEndsStr.second
                << '\n';
    } else {
      std::cout << "replace: " << TypeAndNameSubstitution << "\t from " << DeclRangeEndsStr.first << " to " << DeclRangeEndsStr.second
                << '\n';
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

  if (ICE->getCastKind() == CastKind::CK_LValueToRValue && ICE->getType().getTypePtr()->isFundamentalType()) {
    // assuming all fundamental types are already Safe_T

    const DeclRefExpr* UnderlyingDRE = dyn_cast<DeclRefExpr>(ICE->getSubExpr());

    if (UnderlyingDRE != nullptr /* && UnderlyingDRE->refersToEnclosingVariableOrCapture() */) {
      LocationRange UnderlyingDRERange = GetLocationRange(UnderlyingDRE->getSourceRange(), Context->getSourceManager());
      UnderlyingDRERange.second.second += 1;
      // TODO: check if this line should be two lines below
      // TODO: this part will be refactored anyway
      std::cout << "insert: " << '.' << UB_UninitSafeTypeConsts::GETMETHOD_NAME << "( [[ file contents from "
                << LocPairToString(UnderlyingDRERange.first) << " to " << LocPairToString(UnderlyingDRERange.second) << " ]] ) \t at "
                << LocPairToString(UnderlyingDRERange.second) << '\n';
    }

    // ! problems with Parent-related documentation
    // ? check if a node is an argument in function call ?
    // ? then in 'bad' function call ?

    // for (DynTypedNode& Par : Context->getParents(ICE)) {
    //   CallExpr* FuncCall = Par.get<CallExpr>();
    //   if (FuncCall == nullptr || FuncCall->getDirectCallee() == nullptr)
    //     continue;
    // }
  }

  return true;
}

// detect Safe_T definitions; substitute with .init() function
bool FindSafeTypeDefinitionsVisitor::VisitBinaryOperator(BinaryOperator* BinOp) {
  if (!Context->getSourceManager().isInMainFile(BinOp->getBeginLoc()))
    return true;

  if (BinOp->isAssignmentOp() && BinOp->getLHS()->getType().getTypePtr()->isFundamentalType()) {
    // assuming all fundamental types are already Safe_T
    LocationRange DefinitionExprLocationRange = GetLocationRange(BinOp->getRHS()->getSourceRange(), Context->getSourceManager());

    std::string substitution = '.' + UB_UninitSafeTypeConsts::INITMETHOD_NAME + "( [[ file contents from " +
                               LocPairToString(DefinitionExprLocationRange.first) + " to " +
                               LocPairToString(DefinitionExprLocationRange.second) + " ]] )";
    LocationRange substitutionRange(GetLocationPair(BinOp->getLHS()->getEndLoc(), Context->getSourceManager()),
                                    GetLocationPair(BinOp->getRHS()->getEndLoc(), Context->getSourceManager()));
    substitutionRange.first.second += 1;
    std::cout << "replace: " << substitution << "\t from " << LocPairToString(substitutionRange.first) << " to "
              << LocPairToString(substitutionRange.second) << '\n';
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
