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

CArrayHandler::CArrayHandler(ASTContext* Context) : Context(Context) {}

bool CArrayHandler::VisitFunctionDecl(FunctionDecl* fd) {
  if (Context->getSourceManager().isInMainFile(fd->getBeginLoc())) {
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
    Array_.Sizes_.push_back(getExprAsString(Type->getSizeExpr(), Context));
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
    Array_.InitList_ = getExprAsString(List, Context);
  }
  return true;
}

bool CArrayHandler::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(
        Array_.Sizes_.begin(), std::to_string(Literal->getLength() + 1));
    Array_.hasInitList_ = true;
    Array_.InitList_ = getExprAsString(Literal, Context);
  }
  return true;
}

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
} // namespace

std::string CArrayHandler::generateSafeType(VarDecl* VDecl) {
  std::stringstream NewDecl;
  if (VDecl->isStaticLocal()) {
    NewDecl << "static ";
  }
  generateTemplateType(NewDecl, Array_.Type_, Array_.Sizes_.size() - 1);
  return NewDecl.str();
}

bool CArrayHandler::TraverseVarDecl(VarDecl* VDecl) {
  if (Context->getSourceManager().isInMainFile(VDecl->getBeginLoc())) {
    auto Type = VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = Type->isArrayType();
    if (Type && Array_.shouldVisitNodes_) {
      RecursiveASTVisitor<CArrayHandler>::TraverseVarDecl(VDecl);
      SourceLocation EndLoc =
          findLocAfterRSquare(VDecl, Context, Array_.isIncompleteType_);
      EndLoc.dump(Context->getSourceManager());
      Array_.Name_ = VDecl->getName().str();
      llvm::outs() << generateSafeType(VDecl) << ' ' << Array_.Name_ << "("
                   << getSizes(Array_.Sizes_);
      if (Array_.hasInitList_) {
        llvm::outs() << ", " << Array_.InitList_;
      }
      llvm::outs() << ")\n";
      // "% %" -> UBSafeCArray<%> %(Size)
      // "% %=%" -> UBSafeCArray<%> %(Size, %)
    }
    Array_.reset();
  }
  return true;
}

} // namespace ub_tester
