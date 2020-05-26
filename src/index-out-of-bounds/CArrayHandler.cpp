#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "index-out-of-bounds/CArrayHandler.h"
#include "index-out-of-bounds/IOBStringView.h"

#include <optional>
#include <sstream>
#include <stdio.h>

// TODO Handle templates instations
// TODO Handle taking address of array

using namespace clang;

namespace ub_tester {

void CArrayHandler::ArrayInfo_t::reset() {
  Init_ = std::nullopt;
  shouldVisitNodes_ = isIncompleteType_ = false;
  shouldVisitImplicitCode_ = false;
  Dimension_ = 0;
  Sizes_.clear();
}

CArrayHandler::CArrayHandler(ASTContext* Contex_) : Context_(Contex_) {
  Array_.reset();
}

bool CArrayHandler::shouldVisitImplicitCode() {
  return Array_.shouldVisitImplicitCode_;
}

bool CArrayHandler::VisitConstantArrayType(ConstantArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    int StdBase = 10;
    Array_.Sizes_.push_back(
        Type->getSize().toString(StdBase, false)); // llvm::APInt demands base
  }
  return true;
}

bool CArrayHandler::VisitVariableArrayType(VariableArrayType* Type) {
  if (Array_.shouldVisitNodes_)
    Array_.Sizes_.push_back(getExprAsString(Type->getSizeExpr(), Context_));
  return true;
}

bool CArrayHandler::VisitIncompleteArrayType(IncompleteArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    Array_.isIncompleteType_ = true;
  }
  return true;
}

bool CArrayHandler::VisitInitListExpr(InitListExpr* List) {
  Array_.shouldVisitImplicitCode_ = true;
  if (not Context_->getSourceManager().isWrittenInMainFile(List->getBeginLoc()))
    return true;

  if (List->isSemanticForm() && List->isSyntacticForm()) {
    SubstitutionASTWrapper(Context_)
        .setLoc(List->getBeginLoc())
        .setFormats("@", "{@}")
        .setArguments(List)
        .apply();
  }

  if (Array_.shouldVisitNodes_) {
    if (Array_.isIncompleteType_) {
      Array_.Sizes_.insert(Array_.Sizes_.begin(),
                           std::to_string(List->getNumInits()));
      Array_.isIncompleteType_ = false;
    }

    // Cause of inner InitLists and StringLiterals
    if (!Array_.Init_.has_value())
      Array_.Init_ = getExprAsString(List, Context_);
  }
  return true;
}

bool CArrayHandler::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(Array_.Sizes_.begin(),
                         std::to_string(Literal->getLength() + 1));
    Array_.Init_ = getExprAsString(Literal, Context_);
  }
  return true;
}

std::pair<std::string, std::string> CArrayHandler::getCtorFormats() {
  std::string SourceFormat = Array_.Init_.has_value() ? "#@" : "";
  std::stringstream OutputFormat;
  OutputFormat << "("
               << iob::view::generateSafeArrayCtor(Array_.Sizes_,
                                                   Array_.Init_.has_value()
                                                       ? std::optional("@")
                                                       : std::nullopt)
               << ")";
  return {SourceFormat, OutputFormat.str()};
}

void CArrayHandler::executeSubstitutionOfCtor(VarDecl* D) {
  SourceLocation Loc = getAfterNameLoc(D, Context_);
  std::pair<std::string, std::string> Formats = getCtorFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(Array_.Init_)
      .apply();
}

bool CArrayHandler::TraverseVarDecl(VarDecl* D) {
  if (!Context_->getSourceManager().isWrittenInMainFile(D->getBeginLoc()))
    return true;

  Array_.shouldVisitNodes_ =
      !isa<ParmVarDecl>(D) && D->getType().getTypePtr()->isArrayType();
  RecursiveASTVisitor<CArrayHandler>::TraverseVarDecl(D);
  if (Array_.shouldVisitNodes_) {
    executeSubstitutionOfCtor(D);
  }
  Array_.reset();
  return true;
}

std::pair<std::string, std::string> CArrayHandler::getSubscriptFormats() {
  return {"@[@]", iob::view::generateIOBChecker("@", "@")};
}

void CArrayHandler::executeSubstitutionOfSubscript(
    ArraySubscriptExpr* SubscriptExpr) {
  SourceLocation BeginLoc = SubscriptExpr->getBeginLoc();
  std::pair<std::string, std::string> Formats = getSubscriptFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(BeginLoc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(SubscriptExpr->getLHS(), SubscriptExpr->getRHS())
      .apply();
}

bool CArrayHandler::VisitArraySubscriptExpr(ArraySubscriptExpr* SubscriptExpr) {

  if (Context_->getSourceManager().isWrittenInMainFile(
          SubscriptExpr->getBeginLoc())) {
    executeSubstitutionOfSubscript(SubscriptExpr);
  }
  return true;
}

} // namespace ub_tester
