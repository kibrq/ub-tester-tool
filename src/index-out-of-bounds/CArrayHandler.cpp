#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

#include "index-out-of-bounds/CArrayHandler.h"

#include <sstream>
#include <stdio.h>

using namespace clang;

namespace ub_tester {

void CArrayHandler::ArrayInfo_t::reset() {
  hasInitList_ = shouldVisitNodes_ = isIncompleteType_ = false;
  Sizes_.clear();
}

CArrayHandler::CArrayHandler(ASTContext* Contex_) : Context_(Contex_) {}

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
    Array_.Type_ = Type->getElementType().getUnqualifiedType().getAsString();
    if (Array_.Type_.compare("const char *") == 0) {
      Array_.Type_ = "char *";
      Array_.isConst_ = true;
    } else {
      Array_.isConst_ = Type->getElementType().isConstQualified() ||
                        Type->getElementType().isLocalConstQualified() ||
                        Type->getElementType().isConstant(*Context_);
    }
  }
  return true;
}

bool CArrayHandler::VisitConstantArrayType(ConstantArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    int StdBase = 10;
    Array_.Sizes_.push_back(Type->getSize().toString(StdBase, false));
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
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(
        Array_.Sizes_.begin(), std::to_string(List->getNumInits()));
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
std::string getSubstitutionType(bool isStatic, bool isConst, size_t Dimension);

std::string getSizes(const std::vector<std::string>& Sizes);

std::pair<std::string, std::string> getDeclFormats(
    const std::string& Type, const std::string& Sizes, bool hasInitList,
    const std::string& InitList);

std::vector<std::string> getDeclArgs(
    const std::string& Type, const std::string& Name, bool hasInitList,
    const std::string& InitList);

} // namespace

// Expects that if static and const used together, static is before const
void CArrayHandler::executeSubstitutionOfDecl(VarDecl* ArrayDecl) {
  SourceLocation BeginLoc = ArrayDecl->getBeginLoc();
  std::string generatedType = getSubstitutionType(
      ArrayDecl->isStaticLocal(), Array_.isConst_, Array_.Sizes_.size());
  std::string generatedSizes = getSizes(Array_.Sizes_);
  std::pair<std::string, std::string> Formats = getDeclFormats(
      generatedType, generatedSizes, Array_.hasInitList_, Array_.InitList_);
  std::vector<std::string> Args = getDeclArgs(
      Array_.Type_, Array_.Name_, Array_.hasInitList_, Array_.InitList_);
  BeginLoc.dump(Context_->getSourceManager());
  llvm::outs() << "SourceFormat: " << Formats.first << "\n"
               << "OutputFormat: " << Formats.second << '\n';
}

bool CArrayHandler::TraverseVarDecl(VarDecl* VDecl) {
  if (Context_->getSourceManager().isInMainFile(VDecl->getBeginLoc())) {
    auto Type = VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = Type->isArrayType();
    if (Type && Array_.shouldVisitNodes_) {
      RecursiveASTVisitor<CArrayHandler>::TraverseVarDecl(VDecl);
      Array_.Name_ = VDecl->getName().str();
      executeSubstitutionOfDecl(VDecl);
    }
    Array_.reset();
  }
  return true;
}

void CArrayHandler::executeSubstitutionOfSubscript(
    ArraySubscriptExpr* SubscriptExpr) {}

bool CArrayHandler::VisitArraySubscriptExpr(ArraySubscriptExpr* SubscriptExpr) {
  if (Context_->getSourceManager().isInMainFile(SubscriptExpr->getBeginLoc())) {
    auto BaseType = SubscriptExpr->getBase()->getType().getTypePtrOrNull();
    if (BaseType && BaseType->isArrayType()) {
      executeSubstitutionOfSubscript(SubscriptExpr);
    }
  }
  return true;
}

} // namespace ub_tester

namespace ub_tester {

namespace {
SourceLocation findLocAfterRSquare(
    VarDecl* VDecl, ASTContext* Context, bool isIncompleteType) {

  SourceLocation CurLoc = VDecl->getLocation(),
                 LastValid = VDecl->getLocation();
  SourceManager& SM = Context->getSourceManager();
  LangOptions LO = Context->getLangOpts();

  bool flag = true;
  while (flag) {
    tok::TokenKind TKind =
        Lexer::findNextToken(CurLoc, SM, LO).getValue().getKind();
    if (TKind == tok::semi || TKind == tok::equal) {
      flag = false;
    }
    CurLoc = Lexer::findNextToken(CurLoc, SM, LO).getValue().getLocation();
    if (TKind == tok::r_square) {
      LastValid = CurLoc;
    }
  }
  return Lexer::getLocForEndOfToken(LastValid, 0, SM, LO);
}

void generateTemplateType(
    std::stringstream& Stream, const std::string& Type, int MaxDepth,
    int CurDepth = 0) {
  Stream << "UBSafeCArray<";
  if (CurDepth == MaxDepth) {
    Stream << Type;
  } else {
    generateTemplateType(Stream, Type, MaxDepth, CurDepth + 1);
  }
  Stream << ">";
}

std::string getSubstitutionType(bool isStatic, bool isConst, size_t Dimension) {
  std::stringstream Type;
  Type << (isStatic ? "static " : "");
  for (size_t i = 0; i < Dimension; i++) {
    Type << "UBSafeCArray<";
  }
  Type << (isConst ? "const " : "");
  Type << "@";
  for (size_t i = 0; i < Dimension; i++) {
    Type << ">";
  }
  return Type.str();
}

std::string getSizes(const std::vector<std::string>& Sizes) {
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
    const std::string& Type, const std::string& Sizes, bool hasInitList,
    const std::string& InitList) {
  std::stringstream SourceFormat, OutputFormat;
  SourceFormat << "#@#@#" << (hasInitList ? "=@" : "");
  OutputFormat << Type << " "
               << "@(" << Sizes << (hasInitList ? ", " + InitList : "") << ")";
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

} // namespace
} // namespace ub_tester
