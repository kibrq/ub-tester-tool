#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "index-out-of-bounds/CArrayHandler.h"
#include "index-out-of-bounds/IOBStringView.h"

#include <optional>
#include <sstream>
#include <stdio.h>

// TODO Handle templates instations
// TODO Handle fields
// TODO Handle taking address of array

using namespace clang;

namespace ub_tester {

void CArrayHandler::ArrayInfo_t::reset() {
  Name_ = Type_ = LowestLevelPointeeType_ = InitList_ = std::nullopt;
  shouldVisitNodes_ = isIncompleteType_ = false;
  Dimension_ = 0;
  Sizes_.clear();
}

CArrayHandler::CArrayHandler(ASTContext* Contex_) : Context_(Contex_) { Array_.reset(); }

bool CArrayHandler::VisitArrayType(ArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    ++Array_.Dimension_;
    PrintingPolicy pp(Context_->getLangOpts());
    if (Type->getElementType()->isPointerType()) {
      Array_.LowestLevelPointeeType_ =
          getLowestLevelPointeeType(Type->getElementType()).getAsString(pp);
    } else {
      Array_.Type_ = Type->getElementType().getUnqualifiedType().getAsString(pp);
    }
  }
  return true;
}

bool CArrayHandler::VisitConstantArrayType(ConstantArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    int StdBase = 10;
    Array_.Sizes_.push_back(Type->getSize().toString(StdBase, false)); // llvm::APInt demands base
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
  if (Array_.shouldVisitNodes_) {
    if (Array_.isIncompleteType_) {
      Array_.Sizes_.insert(Array_.Sizes_.begin(), std::to_string(List->getNumInits()));
    }

    // Cause of inner InitLists and StringLiterals
    Array_.shouldVisitNodes_ = false;
    Array_.InitList_ = getExprAsString(List, Context_);
  }
  return true;
}

bool CArrayHandler::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(Array_.Sizes_.begin(), std::to_string(Literal->getLength() + 1));
    Array_.InitList_ = getExprAsString(Literal, Context_);
  }
  return true;
}

namespace {

std::string getCuttedPointerTypeAsString(const std::string& PointeeType, SourceLocation BeginLoc,
                                         ASTContext*);
} // namespace

std::pair<std::string, std::string> CArrayHandler::getDeclFormats(bool isStatic, bool needCtor,
                                                                  char EndSymb) {
  std::stringstream SourceFormat, OutputFormat;
  SourceFormat << "#@#@#" << (Array_.InitList_.has_value() && needCtor ? "@" : "") << EndSymb;
  OutputFormat << iob_view::generateSafeArrayTypename(isStatic, Array_.Dimension_, "@") << " @";
  if (needCtor)
    OutputFormat << "("
                 << iob_view::generateSafeArrayCtor(Array_.Sizes_,
                                                    Array_.InitList_.has_value()
                                                        ? std::optional<std::string>("@")
                                                        : std::nullopt)
                 << ")";

  OutputFormat << EndSymb;
  return {SourceFormat.str(), OutputFormat.str()};
}

void CArrayHandler::executeSubstitutionOfArrayDecl(SourceLocation BeginLoc, bool isStatic,
                                                   bool needCtor, char EndSymb) {

  std::pair<std::string, std::string> Formats = getDeclFormats(isStatic, needCtor, EndSymb);
  if (not Array_.Type_.has_value()) {
    Array_.Type_ =
        getCuttedPointerTypeAsString(Array_.LowestLevelPointeeType_.value(), BeginLoc, Context_);
  }

  ASTFrontendInjector::getInstance().substitute(Context_, BeginLoc, Formats.first, Formats.second,
                                                Array_.Type_, Array_.Name_, Array_.InitList_);
}

void CArrayHandler::executeSubstitutionOfArrayDecl(VarDecl* ArrayDecl) {
  executeSubstitutionOfArrayDecl(ArrayDecl->getBeginLoc(), ArrayDecl->isStaticLocal(), true, ';');
}

bool CArrayHandler::TraverseVarDecl(VarDecl* VDecl) {
  if (Context_->getSourceManager().isInMainFile(VDecl->getBeginLoc())) {
    auto Type = VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = Type->isArrayType();
    RecursiveASTVisitor<CArrayHandler>::TraverseVarDecl(VDecl);
    if (Type && Type->isArrayType()) {
      Array_.Name_ = VDecl->getName().str();
      executeSubstitutionOfArrayDecl(VDecl);
    }
    Array_.reset();
  }
  return true;
}

void CArrayHandler::executeSubstitutionOfArrayDecl(ParmVarDecl* ArrayDecl, char EndSymb) {
  executeSubstitutionOfArrayDecl(ArrayDecl->getBeginLoc(), false, false, EndSymb);
}

bool CArrayHandler::VisitFunctionDecl(FunctionDecl* FDecl) {
  if (Context_->getSourceManager().isWrittenInMainFile(FDecl->getBeginLoc())) {
    for (const auto& Parm : FDecl->parameters()) {
      auto Type = Parm->getOriginalType().getTypePtrOrNull();
      Array_.shouldVisitNodes_ = Type && Type->isArrayType();
      if (Array_.shouldVisitNodes_) {
        RecursiveASTVisitor<CArrayHandler>::TraverseParmVarDecl(Parm);
        Array_.Name_ = Parm->getName().str();
        executeSubstitutionOfArrayDecl(
            Parm, FDecl->getNumParams() - 1 == Parm->getFunctionScopeIndex() ? ')' : ',');
      }
      Array_.reset();
    }
  }
  return true;
}

std::pair<std::string, std::string> CArrayHandler::getSubscriptFormats() {
  return {"@[@]", iob_view::generateIOBChecker("@", "@")};
}

void CArrayHandler::executeSubstitutionOfSubscript(ArraySubscriptExpr* SubscriptExpr) {
  SourceLocation BeginLoc = SubscriptExpr->getBeginLoc();
  std::pair<std::string, std::string> Formats = getSubscriptFormats();
  ASTFrontendInjector::getInstance().substitute(Context_, SubscriptExpr->getBeginLoc(),
                                                Formats.first, Formats.second,
                                                SubscriptExpr->getLHS(), SubscriptExpr->getRHS());
}

bool CArrayHandler::VisitArraySubscriptExpr(ArraySubscriptExpr* SubscriptExpr) {

  if (Context_->getSourceManager().isInMainFile(SubscriptExpr->getBeginLoc())) {
    executeSubstitutionOfSubscript(SubscriptExpr);
  }
  return true;
}

namespace {

std::string getCuttedPointerTypeAsString(const std::string& PointeeType, SourceLocation BeginLoc,
                                         ASTContext* Context) {
  SourceLocation Begin = BeginLoc, End = BeginLoc, CurLoc = BeginLoc;
  SourceManager& SM = Context->getSourceManager();
  const LangOptions& LO = Context->getLangOpts();
  bool flag = true;
  while (flag) {
    auto Tok = Lexer::findNextToken(CurLoc, SM, LO);
    assert(Tok.hasValue());
    if (Tok.getValue().is(tok::raw_identifier)) {
      if (Tok.getValue().getRawIdentifier().str().compare(PointeeType) == 0) {
        Begin = Tok.getValue().getLocation();
      }
    }
    if (Tok.getValue().is(tok::star)) {
      End = Tok.getValue().getEndLoc();
    }
    if (Tok.getValue().isOneOf(tok::semi, tok::equal)) {
      flag = false;
    }
    CurLoc = Tok.getValue().getLocation();
  }

  return Lexer::getSourceText(CharSourceRange::getCharRange(Begin, End), SM, LO).str();
}

} // namespace
} // namespace ub_tester
