#include "arithmetic-overflow/FindArithmeticUBVisitor.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>

using namespace clang;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

namespace ub_tester {

FindArithmeticUBVisitor::FindArithmeticUBVisitor(ASTContext* Context)
    : Context_(Context) {}

bool FindArithmeticUBVisitor::VisitBinaryOperator(BinaryOperator* Binop) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Binop->getBeginLoc()))
    return true;

  QualType BinopType = Binop->getType();
  if (BinopType->isDependentType())
    return true; // templates are not supported yet
  if (BinopType->isFloatingType())
    return true; // floating point types are not supported yet
  if (!BinopType->isFundamentalType())
    return true; // only fundamental type arithmetic is supported

  std::string OperationName = "undefined";
  switch (Binop->getOpcode()) {
  case BinaryOperator::Opcode::BO_Add:
    OperationName = "Sum";
    break;
  case BinaryOperator::Opcode::BO_Sub:
    OperationName = "Diff";
    break;
  case BinaryOperator::Opcode::BO_Mul:
    OperationName = "Mul";
    break;
  case BinaryOperator::Opcode::BO_Div:
    OperationName = "Div";
    break;
  case BinaryOperator::Opcode::BO_Rem:
    OperationName = "Mod";
    break;
  case BinaryOperator::Opcode::BO_Shl:
    OperationName = "BitShiftLeft";
    break;
  case BinaryOperator::Opcode::BO_Shr:
    OperationName = "BitShiftRight";
    break;
  default:
    return true;
  }

  // check BinopType assumption
  assert(!BinopType.hasQualifiers());
  // remove any typedefs
  BinopType = BinopType.getCanonicalType();

  Expr* Lhs = Binop->getLHS();
  Expr* Rhs = Binop->getRHS();
  QualType LhsType = Lhs->getType().getUnqualifiedType().getCanonicalType();
  QualType RhsType = Rhs->getType().getUnqualifiedType().getCanonicalType();
  // only fundamental type arithmetic is supported
  if (!(LhsType->isFundamentalType() && RhsType->isFundamentalType()))
    return true; // some operations can make BinopType fundamental
                 // though lhs or rhs are pointer type

  assert(LhsType == BinopType);
  // lhs and rhs of bitshift operators can have different integer types
  assert(Binop->isShiftOp() || LhsType == RhsType);

  // in binary operators integer promotion is always applied on bool;
  // so _Bool (bool C-type-alias) won't occur
  assert((!LhsType->isBooleanType()) && (!RhsType->isBooleanType()));

  SubstitutionASTWrapper(Context_)
      .setLoc(Binop->getBeginLoc())
      .setPrior(SubstPriorityKind::Shallow)
      .setFormats("@#@", "ASSERT_BINOP(" + OperationName + ", @, @, " +
                             LhsType.getAsString() + ", " +
                             RhsType.getAsString() + ")")
      .setArguments(Lhs, Rhs)
      .apply();

  return true;
}

bool FindArithmeticUBVisitor::VisitUnaryOperator(UnaryOperator* Unop) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Unop->getBeginLoc()))
    return true;
  /*if (!Unop->canOverflow())
    return true;*/ // can't use canOverflow(), it causes ignored warnings

  QualType UnopType = Unop->getType();
  if (UnopType->isDependentType())
    return true; // templates are not supported yet
  if (UnopType->isFloatingType())
    return true; // floating point types are not supported yet
  if (!UnopType->isFundamentalType())
    return true; // only fundamental type arithmetic is supported

  std::string UnopName = UnaryOperator::getOpcodeStr(Unop->getOpcode()).str();
  std::string OperationName = "undefined";
  switch (Unop->getOpcode()) {
  case UnaryOperator::Opcode::UO_Minus:
    OperationName = "UnaryNeg";
    break;
  case UnaryOperator::Opcode::UO_PreInc:
    OperationName = "PrefixIncr";
    break;
  case UnaryOperator::Opcode::UO_PostInc:
    OperationName = "PostfixIncr";
    break;
  case UnaryOperator::Opcode::UO_PreDec:
    OperationName = "PrefixDecr";
    break;
  case UnaryOperator::Opcode::UO_PostDec:
    OperationName = "PostfixDecr";
    break;
  default:
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
  assert(!UnopType->isBooleanType());

  SubstitutionASTWrapper(Context_)
      .setLoc(Unop->getBeginLoc())
      .setPrior(SubstPriorityKind::Shallow)
      .setFormats(Unop->isPrefix() ? UnopName + "#@" : "@#" + UnopName,
                  "ASSERT_UNOP(" + OperationName + ", @, " +
                      UnopType.getAsString() + ")")
      .setArguments(SubExpr)
      .apply();

  return true;
} // namespace ub_tester

bool FindArithmeticUBVisitor::VisitCompoundAssignOperator(
    CompoundAssignOperator* CompAssignOp) {
  if (!Context_->getSourceManager().isWrittenInMainFile(
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

  if (CompAssignOpType->isDependentType())
    return true; // templates are not supported yet
  if (CompAssignOpType->isFloatingType())
    return true; // floating point types are not supported yet
  if (!CompAssignOpType->isFundamentalType())
    return true; // only fundamental type arithmetic is supported
  // should be automatically true for LhsComputationType too
  assert(!LhsComputationType->isDependentType());
  assert(!LhsComputationType->isFloatingType());
  assert(LhsComputationType->isFundamentalType());

  std::string CompAssignOpName = CompAssignOp->getOpcodeStr().str();
  std::string OperationName = "undefined";
  switch (CompAssignOp->getOpcode()) {
  case BinaryOperator::Opcode::BO_AddAssign:
    OperationName = "Sum";
    break;
  case BinaryOperator::Opcode::BO_SubAssign:
    OperationName = "Diff";
    break;
  case BinaryOperator::Opcode::BO_MulAssign:
    OperationName = "Mul";
    break;
  case BinaryOperator::Opcode::BO_DivAssign:
    OperationName = "Div";
    break;
  case BinaryOperator::Opcode::BO_RemAssign:
    OperationName = "Mod";
    break;
  case BinaryOperator::Opcode::BO_ShlAssign:
    OperationName = "BitShiftLeft";
    break;
  case BinaryOperator::Opcode::BO_ShrAssign:
    OperationName = "BitShiftRight";
    break;
  case BinaryOperator::Opcode::BO_AndAssign:
    OperationName = "LogicAnd";
    break;
  case BinaryOperator::Opcode::BO_OrAssign:
    OperationName = "LogicOr";
    break;
  case BinaryOperator::Opcode::BO_XorAssign:
    OperationName = "LogicXor";
    break;
  default:
    return true;
  }

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
  assert(RhsType->isFundamentalType());
  // lhs and rhs of bitshift operators can have different integer types
  assert(CompAssignOp->isShiftAssignOp() || LhsComputationType == RhsType);

  // to prevent _Bool instead of bool type
  std::string LhsTypeName =
      LhsType->isBooleanType() ? "bool" : LhsType.getAsString();
  // other C-type-alias conflicting with C++17 haven't been found yet

  SubstitutionASTWrapper(Context_)
      .setLoc(CompAssignOp->getBeginLoc())
      .setPrior(SubstPriorityKind::Shallow)
      .setFormats("@#@", "ASSERT_COMPASSIGNOP(" + OperationName + ", @, @, " +
                             LhsTypeName + ", " +
                             LhsComputationType.getAsString() + ", " +
                             RhsType.getAsString() + ")")
      .setArguments(Lhs, Rhs)
      .apply();
  return true;
}

bool FindArithmeticUBVisitor::VisitImplicitCastExpr(
    ImplicitCastExpr* ImplicitCast) {
  if (!Context_->getSourceManager().isWrittenInMainFile(
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
  if (ImplicitCastType->isDependentType())
    return true; // templates are not supported yet
  if (ImplicitCastType->isFloatingType())
    return true; // floating point types are not supported yet
  if (!ImplicitCastType->isFundamentalType())
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
  assert(getExprAsString(ImplicitCast, Context_) ==
         getExprAsString(SubExprAsWritten, Context_));
  // check begin location assumption
  assert(ImplicitCast->getBeginLoc() == SubExprAsWritten->getBeginLoc());

  // to prevent _Bool instead of bool type
  std::string ImplicitCastTypeAsString = ImplicitCastType->isBooleanType()
                                             ? "bool"
                                             : ImplicitCastType.getAsString();
  std::string SubExprTypeAsString =
      SubExprType->isBooleanType() ? "bool" : SubExprType.getAsString();
  // other C-type-alias conflicting with C++17 haven't been found yet

  SubstitutionASTWrapper(Context_)
      .setLoc(ImplicitCast->getBeginLoc())
      .setPrior(SubstPriorityKind::Shallow)
      .setFormats("#@", "IMPLICIT_CAST(@, " + SubExprTypeAsString + ", " +
                            ImplicitCastTypeAsString + ")")
      .setArguments(ImplicitCast)
      .apply();
  return true;
}

} // namespace ub_tester
