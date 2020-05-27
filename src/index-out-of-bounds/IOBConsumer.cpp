#include "index-out-of-bounds/IOBConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

IOBConsumer::IOBConsumer(ASTContext* Context) : ArrayVisitor_{Context} {}

void IOBConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  ArrayVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
