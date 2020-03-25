#include "index-out-of-bounds/PointerSubstituter.h"
#include "clang/Basic/SourceManager.h"
#include <cstdio>
#include <unordered_set>

using namespace clang;

namespace {
std::unordered_set<std::string> allocation_functions_names_ = {
    "malloc", "calloc", "realloc", "aligned_alloc"};
}

namespace ub_tester {

PointerSubstituter::PointerSubstituter(ASTContext* Context)
    : Context(Context) {}

bool isPointer(const Type* t) {
  return t && t->isPointerType() && !t->isFunctionPointerType();
}

bool PointerSubstituter::VisitFunctionDecl(FunctionDecl* fd) {
  if (Context->getSourceManager().isInMainFile(fd->getBeginLoc())) {
    for (const auto& param : fd->parameters()) {
      auto type = param->getOriginalType().getTypePtrOrNull();
      if (isPointer(type)) {
        printf("Pointer in functionDecl\n");
      }
    }
    auto return_type = fd->getReturnType().getTypePtrOrNull();
    if (isPointer(return_type)) {
      printf("Pointer in return stmt\n");
    }
  }
  return true;
}

bool PointerSubstituter::VisitPointerType(PointerType* pt) {
  if (pointer_.should_visit_nodes_) {
    pointer_.type_ = pt->getPointeeType().getAsString();
  }
  return true;
}

bool PointerSubstituter::VisitCallExpr(CallExpr* ce) {
  if (pointer_.should_visit_nodes_) {
    if (!Context->getSourceManager().isInMainFile(
            ce->getDirectCallee()->getBeginLoc()))
      printf("%s", ce->getDirectCallee()->getNameInfo().getAsString().c_str());
  }
  return true;
}

bool PointerSubstituter::TraverseVarDecl(VarDecl* vd) {
  if (Context->getSourceManager().isInMainFile(vd->getBeginLoc())) {
    auto type = vd->getType().getTypePtrOrNull();
    pointer_.should_visit_nodes_ = isPointer(type);
    if (pointer_.should_visit_nodes_) {
      RecursiveASTVisitor<PointerSubstituter>::TraverseVarDecl(vd);
      pointer_.name_ = vd->getNameAsString();
    }
    pointer_.should_visit_nodes_ = false;
  }
  return true;
}

bool PointerSubstituter::VisitBinaryOperator(BinaryOperator* bo) {
  return true;
}

} // namespace ub_tester
