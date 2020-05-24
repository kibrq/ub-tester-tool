#include "clang/AST/ASTConsumer.h"

#include "pointers/PointersConsumer.h"

using namespace clang;

namespace ub_tester {

PointersConsumer::PointersConsumer(ASTContext* Context)
    : PointerHandler_{Context} {}

void PointersConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  PointerHandler_.TraverseDecl(Context.getTranslationUnitDecl());
}
} // namespace ub_tester
