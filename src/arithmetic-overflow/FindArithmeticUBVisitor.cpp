#include "arithmetic-overflow/FindArithmeticUBVisitor.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>

using namespace clang;

namespace ub_tester {

FindArithmeticUBVisitor::FindArithmeticUBVisitor(ASTContext* Context)
    : Context(Context) {}

bool FindArithmeticUBVisitor::VisitBinaryOperator(BinaryOperator* Binop) {
  if (!Context->getSourceManager().isWrittenInMainFile(Binop->getBeginLoc()))
    return true;
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
  else if (BinopName == ">>")
    OperationName = "BitShiftRight";
  else
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
  // lhs and rhs of bitshift operators can have different integer types
  assert(BinopName == "<<" || BinopName == ">>" ||
         LhsType.getAsString() == RhsType.getAsString());

  ASTFrontendInjector::getInstance().substitute(
      Context, Binop->getBeginLoc(), "@#" + BinopName + "#@",
      "ASSERT_BINOP(" + OperationName + ", @, @, " + LhsType.getAsString() +
          ", " + RhsType.getAsString() + ")",
      Lhs, Rhs);
  return true;
}

bool FindArithmeticUBVisitor::VisitUnaryOperator(UnaryOperator* Unop) {
  if (!Context->getSourceManager().isWrittenInMainFile(Unop->getBeginLoc()))
    return true;
  /*if (!Unop->canOverflow()) // char c = CHAR_MAX; c++; cannot overflow?
    return true;*/ // can't use canOverflow(), it causes ignored warnings

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
  bool IsPrefixOperator = true;
  if (UnopName == "-") {
    OperationName = "UnaryNeg";
  } else if (UnopName == "++") {
    IsPrefixOperator = Unop->isPrefix();
    OperationName = IsPrefixOperator ? "PrefixIncr" : "PostfixIncr";
  } else if (UnopName == "--") {
    IsPrefixOperator = Unop->isPrefix();
    OperationName = IsPrefixOperator ? "PrefixDecr" : "PostfixDecr";
  } else {
    if (Unop->canOverflow())
      llvm_unreachable("Not known unary operator can overflow");
    return true;
  }

  // check UnopType assumption
  assert(!UnopType.hasQualifiers());
  // remove any typedefs
  UnopType = UnopType.getCanonicalType();

  Expr* SubExpr = Unop->getSubExpr();
  QualType SubExprType =
      SubExpr->getType().getUnqualifiedType().getCanonicalType();
  assert(SubExprType.getAsString() == UnopType.getAsString());

  ASTFrontendInjector::getInstance().substitute(
      Context, Unop->getBeginLoc(),
      IsPrefixOperator ? UnopName + "#@" : "@#" + UnopName,
      "ASSERT_UNOP(" + OperationName + ", @, " + UnopType.getAsString() + ")",
      SubExpr);

  return true;
}

} // namespace ub_tester
