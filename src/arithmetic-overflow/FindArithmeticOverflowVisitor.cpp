#include "arithmetic-overflow/FindArithmeticOverflowVisitor.h"
#include "UBUtility.h"
#include "arithmetic-overflow/ArithmeticOverflowAsserts.h"
#include <cassert>

using namespace clang;

namespace ub_tester {

FindArithmeticOverflowVisitor::FindArithmeticOverflowVisitor(
    ASTContext* Context)
    : Context(Context) {}

bool FindArithmeticOverflowVisitor::VisitBinaryOperator(BinaryOperator* Binop) {
  std::string BinopName = Binop->getOpcodeStr().str();
  std::string OperationName = "undefined";
  if (BinopName == "+")
    OperationName = "Sum";
  else if (BinopName == "-")
    OperationName = "Diff";
  else if (BinopName == "*")
    OperationName = "Mul";
  else
    return true;

  QualType BinopType = Binop->getType();
  assert(BinopType.isTrivialType(*Context)); // is FundamentalType?

  Expr* Lhs = Binop->getLHS();
  Expr* Rhs = Binop->getRHS();

  // check Binop description in documentation
  QualType LhsType = Lhs->getType().getUnqualifiedType().getCanonicalType();
  QualType RhsType = Rhs->getType().getUnqualifiedType().getCanonicalType();
  assert(
      LhsType.getAsString() == BinopType.getAsString() &&
      LhsType.getAsString() == RhsType.getAsString());

  llvm::outs() << getExprLineNCol(Binop, Context) << " ASSERT(" << OperationName
               << ", " << getExprAsString(Lhs, Context) << ", "
               << getExprAsString(Rhs, Context) << ", "
               << BinopType.getAsString() << ");\n";

  return true;
}

} // namespace ub_tester
