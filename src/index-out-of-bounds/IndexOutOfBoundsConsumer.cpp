#include "clang/AST/ASTConsumer.h"

#include "index-out-of-bounds/IndexOutOfBoundsConsumer.h"

using namespace clang;

namespace ub_tester {

IndexOutOfBoundsConsumer::IndexOutOfBoundsConsumer(ASTContext* Context)
    : ArrayHandler_{Context}, pointer_sub_{Context} {}

void IndexOutOfBoundsConsumer::HandleTranslationUnit(
    clang::ASTContext& Context) {
  ArrayHandler_.TraverseDecl(Context.getTranslationUnitDecl());
  pointer_sub_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
