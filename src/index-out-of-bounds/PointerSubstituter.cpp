#include "index-out-of-bounds/PointerSubstituter.h"

using namespace clang;
namespace ub_tester {
PointerSubstituter::PointerSubstituter(ASTContext* Context)
    : Context(Context) {}
bool PointerSubstituter::VisitFunctionDecl(FunctionDecl* fd) { return true; }
bool PointerSubstituter::VisitVarDecl(VarDecl* vd) { return true; }
bool PointerSubstituter::VisitBinaryOperator(BinaryOperator* bo) {
  return true;
}
} // namespace ub_tester
