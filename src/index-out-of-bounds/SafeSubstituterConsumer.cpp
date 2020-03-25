#include "index-out-of-bounds/SafeSubstituterConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

SubstituterConsumer::SubstituterConsumer(ASTContext* Context)
    : array_sub_{Context}, pointer_sub_{Context} {}

void SubstituterConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  array_sub_.TraverseDecl(Context.getTranslationUnitDecl());
  pointer_sub_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
