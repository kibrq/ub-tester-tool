#include "clang/AST/ASTConsumer.h"

#include "index-out-of-bounds/IOBConsumer.h"

using namespace clang;

namespace ub_tester {

IOBConsumer::IOBConsumer(ASTContext* Context) : ArrayHandler_{Context}, PointerHandler_{Context} {}

void IOBConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  ArrayHandler_.TraverseDecl(Context.getTranslationUnitDecl());
  PointerHandler_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
