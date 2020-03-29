#include "index-out-of-bounds/CArraySubstituter.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include <sstream>

#include <stdio.h>

using namespace clang;

namespace ub_tester {

void CArraySubstituter::ArrayInfo_t::reset() {
  shouldVisitNodes_ = isIncompleteType_ = false;
  Sizes_.clear();
}

CArraySubstituter::CArraySubstituter(ASTContext* Context) : Context(Context) {}

bool CArraySubstituter::VisitFunctionDecl(FunctionDecl* fd) {
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

bool CArraySubstituter::VisitArrayType(ArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    Array_.Type_ = Type->getElementType().getAsString();
  }
  return true;
}

bool CArraySubstituter::VisitConstantArrayType(ConstantArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    int StdBase = 10;
    Array_.Sizes_.push_back(Type->getSize().toString(StdBase, false));
  }
  return true;
}

bool CArraySubstituter::VisitVariableArrayType(VariableArrayType* Type) {
  if (Array_.shouldVisitNodes_)
    Array_.Sizes_.push_back(getExprAsString(Type->getSizeExpr(), Context));
  return true;
}

bool CArraySubstituter::VisitIncompleteArrayType(IncompleteArrayType* Type) {
  if (Array_.shouldVisitNodes_) {
    Array_.isIncompleteType_ = true;
  }
  return true;
}

bool CArraySubstituter::VisitInitListExpr(InitListExpr* List) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(
        Array_.Sizes_.begin(), std::to_string(List->getNumInits()));
    Array_.shouldVisitNodes_ = false;
  }
  return true;
}

bool CArraySubstituter::VisitStringLiteral(StringLiteral* Literal) {
  if (Array_.shouldVisitNodes_ && Array_.isIncompleteType_) {
    Array_.Sizes_.insert(
        Array_.Sizes_.begin(), std::to_string(Literal->getLength() + 1));
  }
  return true;
}

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
    std::stringstream& Stream, const std::vector<std::string>& Sizes,
    const std::string& Type, int CurDepth) {
  Stream << "UBSafeCArray<";
  if (CurDepth == Sizes.size() - 1) {
    Stream << Type;
  } else {
    generateTemplateType(Stream, Sizes, Type, CurDepth + 1);
  }
  Stream << ", " << Sizes[CurDepth] << ">";
}

std::string CArraySubstituter::generateSafeType(VarDecl* VDecl) {
  std::stringstream NewDecl;
  if (VDecl->isStaticLocal()) {
    NewDecl << "static ";
  }
  generateTemplateType(NewDecl, Array_.Sizes_, Array_.Type_, 0);
  NewDecl << " " << Array_.Name_;
  return NewDecl.str();
}

bool CArraySubstituter::TraverseVarDecl(VarDecl* VDecl) {
  if (Context->getSourceManager().isInMainFile(VDecl->getBeginLoc())) {
    auto Type = VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = Type->isArrayType();
    if (Type && Array_.shouldVisitNodes_) {
      RecursiveASTVisitor<CArraySubstituter>::TraverseVarDecl(VDecl);
      SourceLocation EndLoc =
          findLocAfterRSquare(VDecl, Context, Array_.isIncompleteType_);
      EndLoc.dump(Context->getSourceManager());
      Array_.Name_ = VDecl->getName().str();
      llvm::outs() << generateSafeType(VDecl) << '\n';
    }
    Array_.reset();
  }
  return true;
}

} // namespace ub_tester
