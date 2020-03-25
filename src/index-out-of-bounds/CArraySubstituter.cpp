#include "index-out-of-bounds/CArraySubstituter.h"
#include "clang/Basic/SourceManager.h"

#include <stdio.h>

using namespace clang;

namespace ub_tester {
CArraySubstituter::CArraySubstituter(ASTContext* Context) : Context(Context) {}

bool CArraySubstituter::VisitFunctionDecl(FunctionDecl* fd) {
  if (Context->getSourceManager().isInMainFile(fd->getBeginLoc())) {
    for (const auto& param : fd->parameters()) {
      auto type = param->getOriginalType().getTypePtrOrNull();
      if (type && type->isPointerType() && !type->isFunctionPointerType()) {
        printf("Pointer in fd\n");
      }
    }
    auto return_type = fd->getReturnType().getTypePtrOrNull();
  }
  return true;
}

bool CArraySubstituter::VisitArrayType(ArrayType* type) {
  if (array_.should_visit_nodes_)
    array_.type_ = type->getElementType().getAsString();
  return true;
}

bool CArraySubstituter::VisitConstantArrayType(ConstantArrayType* type) {
  if (array_.should_visit_nodes_)
    array_.size_ = type->getSize().toString(10, false);
  return true;
}

bool CArraySubstituter::VisitVariableArrayType(VariableArrayType* type) {
  if (array_.should_visit_nodes_)
    array_.size_ = getExprAsString(type->getSizeExpr(), Context);
  return true;
}

bool CArraySubstituter::VisitInitListExpr(InitListExpr* ile) {
  if (array_.should_visit_nodes_)
    array_.size_ = std::to_string(ile->getNumInits());
  return true;
}

bool CArraySubstituter::VisitStringLiteral(StringLiteral* sl) {
  if (array_.should_visit_nodes_)
    array_.size_ = std::to_string(sl->getLength() + 1);
  return true;
}

bool CArraySubstituter::TraverseVarDecl(VarDecl* vd) {
  if (Context->getSourceManager().isInMainFile(vd->getBeginLoc())) {
    auto type = vd->getType().getTypePtrOrNull();
    array_.should_visit_nodes_ = type->isArrayType();
    if (type && array_.should_visit_nodes_) {
      RecursiveASTVisitor<CArraySubstituter>::TraverseVarDecl(vd);
      array_.name_ = vd->getNameAsString();
      printf(
          "UBSafeCArray<%s, %s> %s\n", array_.type_.c_str(),
          array_.size_.c_str(), array_.name_.c_str());
    }
    array_.should_visit_nodes_ = false;
  }
  return true;
}
} // namespace ub_tester
