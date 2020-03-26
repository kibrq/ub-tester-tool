#include "arithmetic-overflow/BinaryOperatorSubsituter.h"
#include <cassert>
#include <sstream>

using namespace clang;

namespace ub_tester {

FindArithmeticOverflowVisitor::FindArithmeticOverflowVisitor(
    ASTContext* Context)
    : Context(Context) {}

bool FindArithmeticOverflowVisitor::VisitBinaryOperator(BinaryOperator* Binop) {
  if (!(Binop->isAdditiveOp() || Binop->isMultiplicativeOp()))
    return true;
  Expr* Lhs = Binop->getLHS();
  Expr* Rhs = Binop->getRHS();
  QualType LhsType = Lhs->getType().getUnqualifiedType();
  QualType RhsType = Rhs->getType().getUnqualifiedType();

  if (!LhsType.isTrivialType(*Context))
    return true;

  assert(RhsType.isTrivialType(*Context));
  assert(LhsType.getAsString() == RhsType.getAsString());
  // llvm::outs() << LhsType.getAsString() << " is trivial\n";

  std::string AssertName = "undefined";
  if (Binop->isAdditiveOp())
    AssertName = "ASSERT_SUM";
  else if (Binop->isMultiplicativeOp())
    AssertName = "ASSERT_MUL";

  llvm::outs() << getExprLineNCol(Binop, Context) << " " << AssertName << "<"
               << LhsType.getAsString() << ">(" << getExprAsString(Lhs, Context)
               << ", " << getExprAsString(Rhs, Context) << ");\n";

  return true;
}

} // namespace ub_tester
