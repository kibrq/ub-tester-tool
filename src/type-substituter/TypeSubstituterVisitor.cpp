#include "type-substituter/TypeSubstituterVisitor.h"

using namespace clang;

namespace ub_tester {

TypeSubstituterVisitor::TypeSubstituterVisitor(ASTContext* Context)
    : Context_{Context} {}

namespace {

void recursiveTypeVisit()

}

bool TypeSubstituterVisitor::VisitValueDecl(ValueDecl* VDecl) {}

} // namespace ub_tester
