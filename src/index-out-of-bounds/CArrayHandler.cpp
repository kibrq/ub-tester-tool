#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

#include "UBUtility.h"
#include "index-out-of-bounds/CArrayHandler.h"

#include <sstream>
#include <stdio.h>

using namespace clang;

namespace ub_tester {

void CArrayHandler::ArrayInfo_t::reset() {
  isElementIsPointer_ = hasInitList_ = shouldVisitNodes_ = isIncompleteType_ =
      false;
  Sizes_.clear();
}

CArrayHandler::CArrayHandler(ASTContext* Contex_) : Context_(Contex_) {
  Array_.reset();
}

bool CArrayHandler::VisitFunctionDecl(FunctionDecl* fd) {
  if (Context_->getSourceManager().isInMainFile(fd->getBeginLoc())) {
    for (const auto& param : fd->parameters()) {
      auto type = param->getOriginalType().getTypePtrOrNull();
      if (type && type->isArrayType()) {
        printf("Array in function declaration\n");
      }
    }
    auto return_type = fd->getReturnType().getTypePtrOrNull();
    if (return_type && return_type->isArrayType()) {
      printf("Array in return stmt\n");
    }
  }
  return true;
}

bool CArrayHandler::VisitArrayType(ArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    if (Type->getElementType()->isPointerType()) {
      Array_.LowestLevelPointeeType_ =
          getLowestLevelPointeeType(Type->getElementType().getTypePtr());
      Array_.isElementIsPointer_ = true;
    } else {
      Array_.Type_ = Type->getElementType().getUnqualifiedType().getAsString();
    }
  }
  return true;
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
  if (Array_.shouldVisitNodes_) {
    if (Array_.isIncompleteType_) {
      Array_.Sizes_.insert(
          Array_.Sizes_.begin(), std::to_string(List->getNumInits()));
    }

    // Cause of inner InitLists and StringLiterals
    Array_.shouldVisitNodes_ = false;
    Array_.hasInitList_ = true;
    Array_.InitList_ = getExprAsString(List, Context_);
  }
  return true;
}

bool CArrayHandler::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(
        Array_.Sizes_.begin(), std::to_string(Literal->getLength() + 1));
    Array_.hasInitList_ = true;
    Array_.InitList_ = getExprAsString(Literal, Context_);
  }
  return true;
}

namespace {

std::string getCuttedPointerTypeAsString(
    const std::string& PointeeType, VarDecl*, ASTContext*);

std::string getSubstituterTypeAsString(bool isStatic, size_t Dimension);

std::string getSizesAsString(const std::vector<std::string>& Sizes);

std::pair<std::string, std::string> getDeclFormats(
    const std::string& Type, const std::string& Sizes, bool hasInitList);

std::vector<std::string> getDeclArgs(
    const std::string& Type, const std::string& Name, bool hasInitList,
    const std::string& InitList);

} // namespace

void CArrayHandler::executeSubstitutionOfDecl(VarDecl* ArrayDecl) {
  SourceLocation BeginLoc = ArrayDecl->getBeginLoc();
  std::string generatedType = getSubstituterTypeAsString(
      ArrayDecl->isStaticLocal(), Array_.Sizes_.size());
  std::string generatedSizes = getSizesAsString(Array_.Sizes_);
  std::pair<std::string, std::string> Formats =
      getDeclFormats(generatedType, generatedSizes, Array_.hasInitList_);
  if (Array_.isElementIsPointer_) {
    Array_.Type_ = getCuttedPointerTypeAsString(
        Array_.LowestLevelPointeeType_, ArrayDecl, Context_);
  }
  std::vector<std::string> Args = getDeclArgs(
      Array_.Type_, Array_.Name_, Array_.hasInitList_, Array_.InitList_);
  llvm::outs() << "SourceFormat: " << Formats.first << "\n"
               << "OutputFormat: " << Formats.second << '\n';
  for (const auto& Arg : Args) {
    llvm::outs() << Arg << ' ';
  }
  llvm::outs() << '\n';
}

bool CArrayHandler::TraverseVarDecl(VarDecl* VDecl) {
  if (Context_->getSourceManager().isInMainFile(VDecl->getBeginLoc())) {
    auto Type = VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = Type->isArrayType();
    RecursiveASTVisitor<CArrayHandler>::TraverseVarDecl(VDecl);
    if (Type && Type->isArrayType()) {
      Array_.Name_ = VDecl->getName().str();
      executeSubstitutionOfDecl(VDecl);
    }
    Array_.reset();
  }
  return true;
}

namespace {
std::pair<std::string, std::string> getSubscriptFormats();
std::vector<std::string> getSubscriptArgs(ArraySubscriptExpr*, ASTContext*);
} // namespace

void CArrayHandler::executeSubstitutionOfSubscript(
    ArraySubscriptExpr* SubscriptExpr) {
  SourceLocation BeginLoc = SubscriptExpr->getBeginLoc();
  std::pair<std::string, std::string> Formats = getSubscriptFormats();
  std::vector<std::string> Args = getSubscriptArgs(SubscriptExpr, Context_);
  llvm::outs() << "SourceFormat: " << Formats.first << "\n"
               << "OutputFormat: " << Formats.second << '\n';
  for (const auto& Arg : Args) {
    llvm::outs() << Arg << ' ';
  }
  llvm::outs() << '\n';
}

bool CArrayHandler::VisitArraySubscriptExpr(ArraySubscriptExpr* SubscriptExpr) {

  if (Context_->getSourceManager().isInMainFile(SubscriptExpr->getBeginLoc())) {
    executeSubstitutionOfSubscript(SubscriptExpr);
  }
  return true;
}

namespace {

std::string getCuttedPointerTypeAsString(
    const std::string& PointeeType, VarDecl* VDecl, ASTContext* Context) {
  SourceLocation Begin = VDecl->getBeginLoc(), End = VDecl->getBeginLoc(),
                 CurLoc = VDecl->getBeginLoc();
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

  return Lexer::getSourceText(CharSourceRange::getCharRange(Begin, End), SM, LO)
      .str();
}

std::string getSubstituterTypeAsString(bool isStatic, size_t Dimension) {
  std::stringstream Type;
  Type << (isStatic ? "static " : "");
  for (size_t i = 0; i < Dimension; i++) {
    Type << "UBSafeCArray<";
  }
  Type << "@";
  for (size_t i = 0; i < Dimension; i++) {
    Type << ">";
  }
  return Type.str();
}

std::string getSizesAsString(const std::vector<std::string>& Sizes) {
  std::stringstream VectorSizes;
  VectorSizes << "{";
  for (size_t i = 0; i < Sizes.size(); i++) {
    VectorSizes << Sizes[i];
    if (i != Sizes.size() - 1) {
      VectorSizes << ", ";
    }
  }
  VectorSizes << "}";
  return VectorSizes.str();
}

std::pair<std::string, std::string> getDeclFormats(
    const std::string& Type, const std::string& Sizes, bool hasInitList) {
  std::stringstream SourceFormat, OutputFormat;
  SourceFormat << "#@#@#" << (hasInitList ? "=@" : "");
  OutputFormat << Type << " "
               << "@(" << Sizes << (hasInitList ? ", @" : "") << ")";
  return {SourceFormat.str(), OutputFormat.str()};
}

std::vector<std::string> getDeclArgs(
    const std::string& Type, const std::string& Name, bool hasInitList,
    const std::string& InitList) {
  std::vector<std::string> Args = {Type, Name};
  if (hasInitList)
    Args.push_back(InitList);
  return Args;
}

std::pair<std::string, std::string> getSubscriptFormats() {
  std::string SourceFormat = "@[@]";
  std::string OutputFormat = "ASSERT(@, @)";
  return {SourceFormat, OutputFormat};
}

std::vector<std::string>
getSubscriptArgs(ArraySubscriptExpr* SubscriptExpr, ASTContext* Context) {
  std::vector<std::string> Args;
  Args.push_back(getExprAsString(SubscriptExpr->getLHS(), Context));
  Args.push_back(getExprAsString(SubscriptExpr->getRHS(), Context));
  return Args;
}

} // namespace
} // namespace ub_tester
