#include "index-out-of-bounds/CArrayVisitor.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "index-out-of-bounds/IOBAssertNames.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include <optional>
#include <sstream>

// TODO Handle templates instations
// TODO Handle taking address of array

using namespace clang;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

namespace ub_tester {

void CArrayVisitor::ArrayInfo_t::reset() {
  Init_ = std::nullopt;
  ShouldVisitNodes_ = IsIncompleteType_ = false;
  ShouldVisitImplicitCode_ = false;
  Dimension_ = 0;
  Sizes_.clear();
}

CArrayVisitor::CArrayVisitor(ASTContext* Context) : Context_(Context) {
  Array_.reset();
}

bool CArrayVisitor::shouldVisitImplicitCode() {
  return Array_.ShouldVisitImplicitCode_;
}

bool CArrayVisitor::VisitConstantArrayType(ConstantArrayType* Type) {
  if (Array_.ShouldVisitNodes_) {
    int StdBase = 10;
    Array_.Sizes_.push_back(
        Type->getSize().toString(StdBase, false)); // llvm::APInt demands base
  }
  return true;
}

bool CArrayVisitor::VisitVariableArrayType(VariableArrayType* Type) {
  if (Array_.ShouldVisitNodes_)
    Array_.Sizes_.push_back(getExprAsString(Type->getSizeExpr(), Context_));
  return true;
}

bool CArrayVisitor::VisitIncompleteArrayType(IncompleteArrayType* Type) {
  if (Array_.ShouldVisitNodes_)
    Array_.IsIncompleteType_ = true;
  return true;
}

bool CArrayVisitor::VisitInitListExpr(InitListExpr* List) {
  Array_.ShouldVisitImplicitCode_ = true;
  if (!Context_->getSourceManager().isWrittenInMainFile(List->getBeginLoc()))
    return true;

  if (List->isSemanticForm() && List->isSyntacticForm())
    SubstitutionASTWrapper(Context_)
        .setLoc(List->getBeginLoc())
        .setFormats("@", "{@}")
        .setArguments(List)
        .apply();

  if (Array_.ShouldVisitNodes_) {
    if (Array_.IsIncompleteType_) {
      Array_.Sizes_.insert(Array_.Sizes_.begin(),
                           std::to_string(List->getNumInits()));
      Array_.IsIncompleteType_ = false;
    }

    // Cause of inner InitLists and StringLiterals
    if (!Array_.Init_.has_value())
      Array_.Init_ = getExprAsString(List, Context_);
  }
  return true;
}

bool CArrayVisitor::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.ShouldVisitNodes_ && Array_.IsIncompleteType_) {
    Array_.Sizes_.insert(Array_.Sizes_.begin(),
                         std::to_string(Literal->getLength() + 1));
    Array_.Init_ = getExprAsString(Literal, Context_);
  }
  return true;
}

std::pair<std::string, std::string> CArrayVisitor::getCtorFormats() {
  std::string SourceFormat = Array_.Init_.has_value() ? "#@" : "";
  std::stringstream OutputFormat;
  OutputFormat << "("
               << iob::names_to_inject::generateSafeArrayCtor(
                      Array_.Sizes_, Array_.Init_.has_value()
                                         ? std::optional("@")
                                         : std::nullopt)
               << ")";
  return {SourceFormat, OutputFormat.str()};
}

void CArrayVisitor::executeSubstitutionOfCtor(VarDecl* VDecl) {
  SourceLocation Loc = getAfterNameLoc(VDecl, Context_);
  std::pair<std::string, std::string> Formats = getCtorFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(Array_.Init_)
      .apply();
}

bool CArrayVisitor::TraverseVarDecl(VarDecl* VDecl) {
  if (!Context_->getSourceManager().isWrittenInMainFile(VDecl->getBeginLoc()))
    return true;
  Array_.ShouldVisitNodes_ =
      !isa<ParmVarDecl>(VDecl) && VDecl->getType()->isArrayType();
  RecursiveASTVisitor<CArrayVisitor>::TraverseVarDecl(VDecl);
  if (Array_.ShouldVisitNodes_) {
    executeSubstitutionOfCtor(VDecl);
  }
  Array_.reset();
  return true;
}

std::pair<std::string, std::string> CArrayVisitor::getSubscriptFormats() {
  return {"@[@]", iob::names_to_inject::generateIOBAssertName("@", "@")};
}

void CArrayVisitor::executeSubstitutionOfSubscript(
    ArraySubscriptExpr* SubscriptExpr) {
  SourceLocation BeginLoc = SubscriptExpr->getBeginLoc();
  std::pair<std::string, std::string> Formats = getSubscriptFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(BeginLoc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(SubscriptExpr->getLHS(), SubscriptExpr->getRHS())
      .apply();
}

bool CArrayVisitor::VisitArraySubscriptExpr(ArraySubscriptExpr* SubscriptExpr) {
  if (!Context_->getSourceManager().isWrittenInMainFile(
          SubscriptExpr->getBeginLoc()))
    return true;
  executeSubstitutionOfSubscript(SubscriptExpr);
  return true;
}

} // namespace ub_tester
