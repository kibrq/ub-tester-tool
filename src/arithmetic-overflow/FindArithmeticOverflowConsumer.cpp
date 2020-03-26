#include "arithmetic-overflow/FindArithmeticOverflowConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

FindArithmeticOverflowConsumer::FindArithmeticOverflowConsumer(
    ASTContext* Context)
    : Visitor{Context} {}

void FindArithmeticOverflowConsumer::HandleTranslationUnit(
    ASTContext& Context) {
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
