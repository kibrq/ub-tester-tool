#include "arithmetic-overflow/FindArithmeticUBConsumer.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

namespace ub_tester {

FindArithmeticUBConsumer::FindArithmeticUBConsumer(ASTContext* Context)
    : Visitor{Context} {}

void FindArithmeticUBConsumer::HandleTranslationUnit(ASTContext& Context) {
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
