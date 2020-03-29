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
  QualType BinopType = Binop->getType();
  const Type* BinopTypePtr = BinopType.getTypePtr();
  if (BinopTypePtr->isDependentType())
    return true; // templates are not supported yet
  if (BinopTypePtr->isFloatingType())
    return true; // floating point types are not supported yet
  if (!BinopTypePtr->isFundamentalType())
    return true; // only fundamental type arithmetic is supported

  std::string BinopName = Binop->getOpcodeStr().str();
  std::string OperationName = "undefined";
  if (BinopName == "+")
    OperationName = "Sum";
  else if (BinopName == "-")
    OperationName = "Diff";
  else if (BinopName == "*")
    OperationName = "Mul";
  else if (BinopName == "/")
    OperationName = "Div";
  else if (BinopName == "%")
    OperationName = "Mod";
  else if (BinopName == "<<")
    OperationName = "BitShiftLeft";
  else // >> is not supported yet
    return true;

  // check BinopType assumption
  assert(!BinopType.hasQualifiers());
  // remove any typedefs
  BinopType = BinopType.getCanonicalType();

  Expr* Lhs = Binop->getLHS();
  Expr* Rhs = Binop->getRHS();
  QualType LhsType = Lhs->getType().getUnqualifiedType().getCanonicalType();
  QualType RhsType = Rhs->getType().getUnqualifiedType().getCanonicalType();
  // only fundamental type arithmetic is supported
  if (!(LhsType.getTypePtr()->isFundamentalType() &&
        RhsType.getTypePtr()->isFundamentalType()))
    return true; // some operations can make BinopType fundamental
                 // though lhs or rhs are pointer type

  assert(LhsType.getAsString() == BinopType.getAsString());
  // rhs of bitshift operators can have different integer type from lhs
  assert(
      BinopName == "<<" || BinopName == ">>" ||
      LhsType.getAsString() == RhsType.getAsString());

  llvm::outs() << getExprLineNCol(Binop, Context) << " ASSERT(" << OperationName
               << ", " << getExprAsString(Lhs, Context) << ", "
               << getExprAsString(Rhs, Context) << ", "
               << BinopType.getAsString() << ");\n";
  return true;
}

bool FindArithmeticOverflowVisitor::VisitUnaryOperator(UnaryOperator* Unop) {
  if (!Unop->canOverflow())
    return true;

  QualType UnopType = Unop->getType();
  const Type* UnopTypePtr = UnopType.getTypePtr();
  if (UnopTypePtr->isDependentType())
    return true; // templates are not supported yet
  if (UnopTypePtr->isFloatingType())
    return true; // floating point types are not supported yet
  if (!UnopTypePtr->isFundamentalType())
    return true; // only fundamental type arithmetic is supported

  std::string UnopName = UnaryOperator::getOpcodeStr(Unop->getOpcode()).str();
  std::string OperationName = "undefined";
  if (UnopName == "-") {
    OperationName = "UnaryNegation";
  } else if (UnopName == "++") {
    OperationName = Unop->isPrefix() ? "PrefixIncrement" : "PostfixIncrement";
  } else if (UnopName == "--") {
    OperationName = Unop->isPrefix() ? "PrefixDecrement" : "PostfixDecrement";
  } else {
    llvm_unreachable("Unknown unary operator can overflow");
  }

  // check UnopType assumption
  assert(!UnopType.hasQualifiers());
  // remove any typedefs
  UnopType = UnopType.getCanonicalType();

  Expr* SubExpr = Unop->getSubExpr();
  QualType SubExprType =
      SubExpr->getType().getUnqualifiedType().getCanonicalType();
  assert(SubExprType.getAsString() == UnopType.getAsString());

  llvm::outs() << getExprLineNCol(Unop, Context) << " ASSERT(" << OperationName
               << ", " << getExprAsString(SubExpr, Context) << ", "
               << UnopType.getAsString() << ");\n";

  return true;
}

} // namespace ub_tester
