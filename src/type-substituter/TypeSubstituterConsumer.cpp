#include "type-substituter/TypeSubstituterConsumer.h"

namespace ub_tester {

TypeSubstituterConsumer::TypeSubstituterConsumer(ASTContext* Context)
    : Substituter_{Context} {}

void TypeSubstituterConsumer::HandleTranslationUnit(ASTContext& Context) {
  Substituter_.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace ub_tester
