#include "pointers/PointersConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

PointersConsumer::PointersConsumer(ASTContext* Context)
    : PointerVisitor_{Context} {}

void PointersConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  PointerVisitor_.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
