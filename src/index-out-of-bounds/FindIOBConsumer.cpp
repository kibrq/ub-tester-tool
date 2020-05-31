#include "index-out-of-bounds/FindIOBConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

FindIOBConsumer::FindIOBConsumer(ASTContext* Context)
    : ArrayVisitor_{Context} {}

void FindIOBConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  ArrayVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
