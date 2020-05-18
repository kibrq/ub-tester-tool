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

  assert(LhsType == BinopType);
  // lhs and rhs of bitshift operators can have different integer types
  assert(BinopName == "<<" || BinopName == ">>" || LhsType == RhsType);

  // in binary operators integer promotion is always applied on bool;
  // so _Bool (bool C-type-alias) won't occur
  assert((!LhsType.getTypePtr()->isBooleanType()) &&
         (!RhsType.getTypePtr()->isBooleanType()));

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
  assert(SubExprType == UnopType);

  // in binary operators integer promotion is always applied on bool;
  // so _Bool (bool C-type-alias) won't occur
  assert(!UnopType.getTypePtr()->isBooleanType());

  ASTFrontendInjector::getInstance().substitute(
      Context, Unop->getBeginLoc(),
      IsPrefixOperator ? UnopName + "#@" : "@#" + UnopName,
      "ASSERT_UNOP(" + OperationName + ", @, " + UnopType.getAsString() + ")",
      SubExpr);
  return true;
}

bool FindArithmeticUBVisitor::VisitCompoundAssignOperator(
    CompoundAssignOperator* CompAssignOp) {
  if (!Context->getSourceManager().isWrittenInMainFile(
          CompAssignOp->getBeginLoc()))
    return true;

  /* for (lhs CompAssignOp rhs): lhs is converted to
   * CompAssignOp->getComputationLHSType(), rhs is converted to
   * CompAssignOp->getRHS()->getType(), operation perfoms, then result is
   * converted to CompAssignOp->getType() (original type of lhs, ==
   * CompAssignOp->getLHS()->getType()) and is written to lhs */
  /* presumably, CompAssignOp->getComputationLHSType() ==
   * CompAssignOp->getComputationResultType() for all fundamental types (and
   * differs for pointer operations) */

  QualType CompAssignOpType = CompAssignOp->getType();
  QualType LhsComputationType = CompAssignOp->getComputationLHSType();
  const Type* CompAssignOpTypePtr = CompAssignOpType.getTypePtr();
  const Type* LhsComputationTypePtr = LhsComputationType.getTypePtr();

  if (CompAssignOpTypePtr->isDependentType())
    return true; // templates are not supported yet
  if (CompAssignOpTypePtr->isFloatingType())
    return true; // floating point types are not supported yet
  if (!CompAssignOpTypePtr->isFundamentalType())
    return true; // only fundamental type arithmetic is supported
  // should be automatically true for LhsComputationType too
  assert(!LhsComputationTypePtr->isDependentType());
  assert(!LhsComputationTypePtr->isFloatingType());
  assert(LhsComputationTypePtr->isFundamentalType());

  std::string CompAssignOpName = CompAssignOp->getOpcodeStr().str();
  std::string OperationName = "undefined";
  if (CompAssignOpName == "+=")
    OperationName = "Sum";
  else if (CompAssignOpName == "-=")
    OperationName = "Diff";
  else if (CompAssignOpName == "*=")
    OperationName = "Mul";
  else if (CompAssignOpName == "/=")
    OperationName = "Div";
  else if (CompAssignOpName == "%=")
    OperationName = "Mod";
  else if (CompAssignOpName == "<<=")
    OperationName = "BitShiftLeft";
  else if (CompAssignOpName == ">>=")
    OperationName = "BitShiftRight";
  else if (CompAssignOpName == "&=")
    OperationName = "LogicAnd";
  else if (CompAssignOpName == "|=")
    OperationName = "LogicOr";
  else if (CompAssignOpName == "^=")
    OperationName = "LogicXor";
  else
    return true;

  // check CompAssignOpType and LhsComputationType assumption
  assert(!CompAssignOpType.hasQualifiers());
  assert(!LhsComputationType.hasQualifiers());
  // remove any typedefs
  CompAssignOpType = CompAssignOpType.getCanonicalType();
  LhsComputationType = LhsComputationType.getCanonicalType();

  Expr* Lhs = CompAssignOp->getLHS();
  Expr* Rhs = CompAssignOp->getRHS();
  QualType LhsType = Lhs->getType().getUnqualifiedType().getCanonicalType();
  QualType RhsType = Rhs->getType().getUnqualifiedType().getCanonicalType();

  // check understanding of CompoundAssignOperator
  assert(LhsType == CompAssignOpType);
  assert(LhsComputationType ==
         CompAssignOp->getComputationResultType().getCanonicalType());
  // should be automatically true because LhsComputationType is fundamental
  assert(RhsType.getTypePtr()->isFundamentalType());
  // lhs and rhs of bitshift operators can have different integer types
  assert(CompAssignOpName == "<<=" || CompAssignOpName == ">>=" ||
         LhsComputationType == RhsType);

  // to prevent _Bool instead of bool type
  std::string LhsTypeName =
      LhsType.getTypePtr()->isBooleanType() ? "bool" : LhsType.getAsString();
  // other C-type-alias conflicting with C++17 haven't been found yet

  ASTFrontendInjector::getInstance().substitute(
      Context, CompAssignOp->getBeginLoc(), "@#" + CompAssignOpName + "#@",
      "ASSERT_COMPASSIGNOP(" + OperationName + ", @, @, " + LhsTypeName + ", " +
          LhsComputationType.getAsString() + ", " + RhsType.getAsString() + ")",
      Lhs, Rhs);
  return true;
}

bool FindArithmeticUBVisitor::VisitImplicitCastExpr(
    ImplicitCastExpr* ImplicitCast) {
  if (!Context->getSourceManager().isWrittenInMainFile(
          ImplicitCast->getBeginLoc()))
    return true;

  switch (ImplicitCast->getCastKind()) {
  case CastKind::CK_IntegralCast:
    break;
  case CastKind::CK_IntegralToBoolean:
    break;
  // case CastKind::CK_BooleanToSignedIntegral:
  // break; // never meet with kind of cast
  default:
    return true; // only integral cast is supported for now
  }
  if (ImplicitCast->isPartOfExplicitCast())
    return true; // explicit cast is considered separately

  QualType ImplicitCastType = ImplicitCast->getType();
  const Type* ImplicitCastTypePtr = ImplicitCastType.getTypePtr();
  if (ImplicitCastTypePtr->isDependentType())
    return true; // templates are not supported yet
  if (ImplicitCastTypePtr->isFloatingType())
    return true; // floating point types are not supported yet
  if (!ImplicitCastTypePtr->isFundamentalType())
    return true; // only fundamental type conversion is supported

  // check ImplicitCastType assumption
  assert(!ImplicitCastType.hasQualifiers());
  // remove any typedefs
  ImplicitCastType = ImplicitCastType.getCanonicalType();

  Expr* SubExpr = ImplicitCast->getSubExpr();
  Expr* SubExprAsWritten = ImplicitCast->getSubExprAsWritten();
  QualType SubExprType =
      SubExpr->getType().getUnqualifiedType().getCanonicalType();
  assert(SubExprType != ImplicitCastType);

  // check heuristic assumption about string-representation
  assert(getExprAsString(ImplicitCast, Context) ==
         getExprAsString(SubExprAsWritten, Context));
  // check begin location assumption
  assert(ImplicitCast->getBeginLoc() == SubExprAsWritten->getBeginLoc());

  // to prevent _Bool instead of bool type
  std::string ImplicitCastTypeAsString = ImplicitCastTypePtr->isBooleanType()
                                             ? "bool"
                                             : ImplicitCastType.getAsString();
  std::string SubExprTypeAsString = SubExprType.getTypePtr()->isBooleanType()
                                        ? "bool"
                                        : SubExprType.getAsString();
  // other C-type-alias conflicting with C++17 haven't been found yet

  ASTFrontendInjector::getInstance().substitute(
      Context, ImplicitCast->getBeginLoc(), "#@",
      "IMPLICIT_CAST(@, " + SubExprTypeAsString + ", " +
          ImplicitCastTypeAsString + ")",
      ImplicitCast);

  return true;
}

} // namespace ub_tester
