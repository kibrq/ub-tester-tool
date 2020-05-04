#include "utility/UtilityConsumer.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

UtilityConsumer::UtilityConsumer(clang::ASTContext* Context) : FuncCodeAvailVisitor(Context) {}

void UtilityConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FuncCodeAvailVisitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester