#include "pointer-ub/FindPointerUBConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

FindPointerUBConsumer::FindPointerUBConsumer(ASTContext* Context)
    : FindPointerUBVisitor_{Context} {}

void FindPointerUBConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FindPointerUBVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
