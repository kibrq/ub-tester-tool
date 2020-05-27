#pragma once

#include "ArithmeticUBCheckers.h"
#include "TypeConversionCheckers.h"
#include "assert-message-manager/AssertMessageManager.h"
#include <cstring>
#include <sstream>

// Lhs and Rhs can be put in Assert-functions as strings to improve error-log
/* Binary operations usually have equal Lhs and Rhs types, but there are some
 * exceptions like bitshift operators (can have different integer types).
 * However, return type of binary operator is always Lhs type. */
#define ASSERT_BINOP(Operation, Lhs, Rhs, LhsType, RhsType)                    \
  ub_tester::arithm::asserts::assert##Operation<LhsType, RhsType>(             \
      (Lhs), (Rhs), #LhsType, __FILE__, __LINE__)
#define ASSERT_UNOP(Operation, Expr, Type)                                     \
  ub_tester::arithm::asserts::assert##Operation<Type>((Expr), #Type, __FILE__, \
                                                      __LINE__)

#define ASSERT_COMPASSIGNOP(Operation, Lhs, Rhs, LhsType, LhsComputationType,  \
                            RhsType)                                           \
  ub_tester::arithm::asserts::assertCompAssignOp##Operation<                   \
      LhsType, LhsComputationType, RhsType>(                                   \
      (Lhs), (Rhs), #LhsType, #LhsComputationType, __FILE__, __LINE__)

#define IMPLICIT_CAST(SubExpr, FromType, ToType)                               \
  ub_tester::arithm::asserts::casts::assertIntegralCast<FromType, ToType>(     \
      (SubExpr), #FromType, #ToType, __FILE__, __LINE__)

#define OVERFLOW_DETECTED(Type, Message)                                       \
  if (std::numeric_limits<Type>::is_signed) {                                  \
    PUSH_ERROR(OVERFLOW_ERROR, (Message));                                     \
  } else                                                                       \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING, (Message))

#define ARE_SAME_TYPES(Type1, Type2)                                           \
  static_assert(std::is_same<Type1, Type2>::value)
#define HAS_CONV_RANK_GEQ_THAN_INT(Type)                                       \
  static_assert(arithm_util::checkIfTypeHasConvRankGeqThanInt<Type>());

namespace ub_tester::arithm::asserts {

using arithm::checkers::ArithmCheckRes;
using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;
using type_conv::TyCoCheckRes;

namespace casts {

template <typename FromType, typename ToType>
ToType assertIntegralCast(FromType SubExpr, const char* FromTypeName,
                          const char* ToTypeName, const char* Filename,
                          int Line) {
  static_assert(std::numeric_limits<FromType>::is_integer);
  static_assert(std::numeric_limits<ToType>::is_integer);
  ToType SubExprInToType = static_cast<ToType>(SubExpr);
  std::stringstream Message;

  switch (
      type_conv::Conversions<FromType, ToType>::checkIntegralConv(SubExpr)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    Message << "unsafe conversion while implicit cast in " << Filename
            << " Line: " << Line << "\nlog: "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName
            << ";\n     negative value to unsigned type conversion does not "
               "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    Message << "unsafe conversion while implicit cast in " << Filename
            << " Line: " << Line << "\nlog: "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName
            << ";\n     res overflows to-type max value: " << +SubExpr << " > "
            << +std::numeric_limits<ToType>::max()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    Message << "unsafe conversion while implicit cast in " << Filename
            << " Line: " << Line << "\nlog: "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName
            << ";\n     res overflows to-type min value: " << +SubExpr << " < "
            << +std::numeric_limits<ToType>::lowest()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    Message << "unsafe conversion while implicit cast in " << Filename
            << " Line: " << Line << "\nlog: "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName
            << ";\n     res overflows to-type max value: " << +SubExpr << " > "
            << +std::numeric_limits<ToType>::max()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    Message << "unsafe conversion while implicit cast in " << Filename
            << " Line: " << Line << "\nlog: "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName
            << ";\n     res overflows to-type min value: " << +SubExpr << " < "
            << +std::numeric_limits<ToType>::lowest()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    Message << "bool-conversion while implicit cast in " << Filename
            << " Line: " << Line
            << "\nlog: bool-conversions are not considered;\n     "
            << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
            << ToTypeName << " " << +SubExprInToType << ") from "
            << FromTypeName << " to " << ToTypeName << "\n";
    PUSH_WARNING(NOT_CONSIDERED_WARNING, Message.str());
    break;

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
  return SubExpr;
}

} // namespace casts

template <typename LhsType, typename RhsType>
LhsType assertSum(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkSum<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " + " << +Rhs << " > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs + Rhs;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " + " << +Rhs << " < "
            << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs + Rhs;

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs + Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiff(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                   const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkDiff<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " - " << +Rhs << " > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs - Rhs;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " - " << +Rhs << " < "
            << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs - Rhs;

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs - Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMul(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkMul<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " * " << +Rhs << " > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs * Rhs;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " * " << +Rhs << " < "
            << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs * Rhs;

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs * Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiv(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  std::stringstream Message;

  // check for flt-point in future: minLim <= (Lhs / 0-approx Rhs) <= maxLim
  switch (arithm::checkers::checkDiv<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " / " << +Rhs << " > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs / Rhs;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: " << +Lhs << " / " << +Rhs << " < "
            << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs / Rhs;

  case ArithmCheckRes::DIV_BY_0:
    Message << LhsTypeName << " division by 0 in " << Filename
            << " Line: " << Line << "\n";
    PUSH_ERROR(DIVISION_BY_ZERO_ERROR, Message.str());

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs / Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiv");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMod(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkMod<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    Message << LhsTypeName << " mod (%) is undefined in " << Filename
            << " Line: " << Line
            << "\nlog: because division is undefined; overflow: " << +Lhs
            << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
            << "\n";
    PUSH_ERROR(UNDEFINED_MOD_ERROR, Message.str());

  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    Message << LhsTypeName << " mod (%) is undefined in " << Filename
            << " Line: " << Line
            << "\nlog: because division is undefined; overflow: " << +Lhs
            << " / " << +Rhs << " < " << +std::numeric_limits<LhsType>::lowest()
            << "\n";
    PUSH_ERROR(UNDEFINED_MOD_ERROR, Message.str());

  case ArithmCheckRes::DIV_BY_0:
    Message << LhsTypeName << " mod (%) by 0 in " << Filename
            << " Line: " << Line << "\n";
    PUSH_ERROR(DIVISION_BY_ZERO_ERROR, Message.str());

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs % Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMod");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftLeft(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                           const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  using UnsignedLhsType = typename std::make_unsigned<LhsType>::type;
  std::stringstream Message;

  switch (arithm::checkers::checkBitShiftLeft<LhsType, RhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    Message << LhsTypeName << " bitshift left (<<) is undefined in " << Filename
            << " Line: " << Line << "\nlog: negative rhs; " << +Rhs << " < 0\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    Message << LhsTypeName << " bitshift left (<<) is undefined in " << Filename
            << " Line: " << Line << "\nlog: rhs >= number of bits in lhs type; "
            << +Rhs << " >= " << +arithm_util::getTypeSizeInBits<LhsType>()
            << "\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

#if __cplusplus > 201703L // only since C++20
  case ArithmCheckRes::OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: (" << +Lhs << " << " << +Rhs << ") > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING, Message.str());
    return Lhs << Rhs;

  case ArithmCheckRes::OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: (" << +Lhs << " << " << +Rhs << ") < "
            << +std::numeric_limits<LhsType>::min() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING, Message.str());
    return Lhs << Rhs;
#endif

  case ArithmCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    Message << LhsTypeName << " bitshift left (<<) is undefined in " << Filename
            << " Line: " << Line << "\nlog: negative lhs; " << +Lhs << " < 0\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS:
    Message << LhsTypeName << " bitshift left (<<) is undefined in " << Filename
            << " Line: " << Line
            << "\nlog: signed lhs is non-negative, but result is not "
               "representable in unsigned version of lhs type; ("
            << +Lhs << " << " << +Rhs << ") > "
            << std::numeric_limits<UnsignedLhsType>::max() << "\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: (" << +Lhs << " << " << +Rhs << ") > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_DETECTED(LhsType, Message.str());
    return Lhs << Rhs;

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs << Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftLeft");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftRight(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                            const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkBitShiftRight<LhsType, RhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    Message << LhsTypeName << " bitshift right (>>) is undefined in "
            << Filename << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
            << " < 0\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_RIGHT_ERROR, Message.str());

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    Message << LhsTypeName << " bitshift right (>>) is undefined in "
            << Filename << " Line: " << Line
            << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
            << " >= " << +arithm_util::getTypeSizeInBits<LhsType>() << "\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_RIGHT_ERROR, Message.str());

  case ArithmCheckRes::IMPL_DEFINED_OPERATION:
    Message << LhsTypeName
            << " bitshift right (>>) is implementation-defined in " << Filename
            << " Line: " << Line << "\nlog: negative lhs; " << +Lhs << " < 0\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING, Message.str());
    return Lhs >> Rhs;

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs >> Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftRight");
  }
}

template <typename T>
T assertUnaryNeg(T Expr, const char* TypeName, const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  HAS_CONV_RANK_GEQ_THAN_INT(T); // integral promotion is expected
  std::stringstream Message;

  switch (arithm::checkers::checkUnaryNeg<T>(Expr)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << TypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: -(" << +Expr << ") > " << +std::numeric_limits<T>::max()
            << "\n";
    OVERFLOW_DETECTED(T, Message.str());
    return -Expr;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << TypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: -(" << +Expr << ") < "
            << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_DETECTED(T, Message.str());
    return -Expr;

  case ArithmCheckRes::SAFE_OPERATION:
    return -Expr;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkUnaryNeg");
  }
}

namespace support {

template <typename Type, typename CommonType>
void assertIncrOrDecrOpResTypeConv(Type Expr, CommonType ComputedOperationRes,
                                   const char* OpName, const char* InnerOpName,
                                   const char* OpAppliedOnExprName,
                                   const char* CompAssignOpAppliedOnExprName,
                                   const char* TypeName,
                                   const char* CommonTypeName,
                                   const char* Filename, int Line) {
  static_assert(std::numeric_limits<Type>::is_integer);
  static_assert(std::numeric_limits<CommonType>::is_integer);
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  std::stringstream Message;

  switch (type_conv::Conversions<CommonType, Type>::checkIntegralConv(
      ComputedOperationRes)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    Message << "unsafe conversion while (" << OpName << ") computation in "
            << Filename << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
            << " is computed as " << CompAssignOpAppliedOnExprName
            << ", i.e. as " << CommonTypeName
            << " expression;\n     conversion of ((" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") "
            << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
            << " from " << CommonTypeName << " to " << TypeName
            << ";\n     negative value to unsigned type conversion does not "
               "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    Message << "unsafe conversion while (" << OpName << ") computation in "
            << Filename << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
            << " is computed as " << CompAssignOpAppliedOnExprName
            << ", i.e. as " << CommonTypeName
            << " expression;\n     conversion of ((" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") "
            << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
            << " from " << CommonTypeName << " to " << TypeName
            << ";\n     res overflows to-type max value: "
            << +ComputedOperationRes << " > "
            << +std::numeric_limits<Type>::max()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    Message << "unsafe conversion while (" << OpName << ") computation in "
            << Filename << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
            << " is computed as " << CompAssignOpAppliedOnExprName
            << ", i.e. as " << CommonTypeName
            << " expression;\n     conversion of ((" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") "
            << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
            << " from " << CommonTypeName << " to " << TypeName
            << ";\n     res overflows to-type min value: "
            << +ComputedOperationRes << " < "
            << +std::numeric_limits<Type>::lowest()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    Message << "unsafe conversion while (" << OpName << ") computation in "
            << Filename << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
            << " is computed as " << CompAssignOpAppliedOnExprName
            << ", i.e. as " << CommonTypeName
            << " expression;\n     conversion of ((" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") "
            << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
            << " from " << CommonTypeName << " to " << TypeName
            << ";\n     res overflows to-type max value: "
            << +ComputedOperationRes << " > "
            << +std::numeric_limits<Type>::max()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    Message << "unsafe conversion while (" << OpName << ") computation in "
            << Filename << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
            << " is computed as " << CompAssignOpAppliedOnExprName
            << ", i.e. as " << CommonTypeName
            << " expression;\n     conversion of ((" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") "
            << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
            << " from " << CommonTypeName << " to " << TypeName
            << ";\n     res overflows to-type min value: "
            << +ComputedOperationRes << " < "
            << +std::numeric_limits<Type>::lowest()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    assert(
        0 &&
        "bool prefix/postfix increment/decrement are deprecated since C++17");

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
}

} // namespace support

/* computation of incr/decr operators applied on x <=> computation of x +=/-= 1:
 * 1) x and 1 are converted to common type of T and int
 * 2) (x +/- 1) is computed
 * 3) res is written to x through conversion to T
 * (of course, postfix operators return copy of x, but point 3) still occurs */

template <typename T>
inline T& assertPrefixIncr(T& Expr, const char* TypeName, const char* Filename,
                           int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  using CommonType = typename std::common_type<T, int>::type;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);
  std::stringstream Message;

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_util::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkSum<CommonType>(ExprInCommonType,
                                                 RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << CommonTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: ++(" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") > "
            << +std::numeric_limits<T>::max()
            << "\n     ++expr is computed as expr += 1, i.e. as "
            << CommonTypeName << " expression\n";
    OVERFLOW_DETECTED(CommonType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Prefix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType + 1, "prefix ++", "+", "++expr", "expr += 1",
      TypeName, CommonTypeName.c_str(), Filename, Line);

  return ++Expr;
}
template <>
inline bool& assertPrefixIncr<bool>(bool& Expr, const char* TypeName,
                                    const char* Filename, int Line) {
  arithm_util::UnusedArgs{Expr, TypeName, Filename, Line};
  assert(0 && "bool prefix increment is deprecated since C++17");
}

template <typename T>
inline T assertPostfixIncr(T& Expr, const char* TypeName, const char* Filename,
                           int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  using CommonType = typename std::common_type<T, int>::type;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);
  std::stringstream Message;

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_util::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkSum<CommonType>(ExprInCommonType,
                                                 RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << CommonTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: (" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ")++ > "
            << +std::numeric_limits<T>::max()
            << "\n     expr++ is computed as expr += 1, i.e. as "
            << CommonTypeName << " expression\n";
    OVERFLOW_DETECTED(CommonType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Postfix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType + 1, "postfix ++", "+", "expr++", "expr += 1",
      TypeName, CommonTypeName.c_str(), Filename, Line);

  return Expr++;
}
template <>
inline bool assertPostfixIncr<bool>(bool& Expr, const char* TypeName,
                                    const char* Filename, int Line) {
  arithm_util::UnusedArgs{Expr, TypeName, Filename, Line};
  assert(0 && "bool postfix increment is deprecated since C++17");
}

template <typename T>
inline T& assertPrefixDecr(T& Expr, const char* TypeName, const char* Filename,
                           int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  using CommonType = typename std::common_type<T, int>::type;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);
  std::stringstream Message;

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_util::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkDiff<CommonType>(ExprInCommonType,
                                                  RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Prefix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << CommonTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: --(" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ") < "
            << +std::numeric_limits<T>::lowest()
            << "\n     --expr is computed as expr -= 1, i.e. as "
            << CommonTypeName << " expression\n";
    OVERFLOW_DETECTED(CommonType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType - 1, "prefix --", "-", "--expr", "expr -= 1",
      TypeName, CommonTypeName.c_str(), Filename, Line);

  return --Expr;
}
template <>
inline bool& assertPrefixDecr<bool>(bool& Expr, const char* TypeName,
                                    const char* Filename, int Line) {
  arithm_util::UnusedArgs{Expr, TypeName, Filename, Line};
  assert(0 && "bool prefix decrement is deprecated since C++17");
}

template <typename T>
inline T assertPostfixDecr(T& Expr, const char* TypeName, const char* Filename,
                           int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  using CommonType = typename std::common_type<T, int>::type;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);
  std::stringstream Message;

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_util::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkDiff<CommonType>(ExprInCommonType,
                                                  RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Postfix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << CommonTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: (" << TypeName << " " << +Expr
            << " -> " << CommonTypeName << " " << +ExprInCommonType << ")-- < "
            << +std::numeric_limits<T>::lowest()
            << "\n     expr-- is computed as expr -= 1, i.e. as "
            << CommonTypeName << " expression\n";
    OVERFLOW_DETECTED(CommonType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType - 1, "postfix --", "-", "expr--", "expr -= 1",
      TypeName, CommonTypeName.c_str(), Filename, Line);

  return Expr--;
}
template <>
inline bool assertPostfixDecr<bool>(bool& Expr, const char* TypeName,
                                    const char* Filename, int Line) {
  arithm_util::UnusedArgs{Expr, TypeName, Filename, Line};
  assert(0 && "bool postfix decrement is deprecated since C++17");
}

namespace support {

template <typename LhsType, typename LhsComputationType>
void checkCompAssignOpResTypeConv(LhsType Lhs, LhsComputationType Rhs,
                                  LhsComputationType ComputedOperationRes,
                                  const char* InnerOpName,
                                  const char* LhsTypeName,
                                  const char* LhsComputationTypeName,
                                  const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (
      type_conv::Conversions<LhsComputationType, LhsType>::checkIntegralConv(
          ComputedOperationRes)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    Message << "unsafe conversion while (" << InnerOpName
            << "=) computation in " << Filename << " Line: " << Line
            << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName
            << ";\n     negative value to unsigned type conversion does not "
               "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    Message << "unsafe conversion while (" << InnerOpName
            << "=) computation in " << Filename << " Line: " << Line
            << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName
            << ";\n     res overflows to-type max value: "
            << +ComputedOperationRes << " > "
            << +std::numeric_limits<LhsType>::max()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    Message << "unsafe conversion while (" << InnerOpName
            << "=) computation in " << Filename << " Line: " << Line
            << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName
            << ";\n     res overflows to-type min value: "
            << +ComputedOperationRes << " < "
            << +std::numeric_limits<LhsType>::lowest()
            << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    Message << "unsafe conversion while (" << InnerOpName
            << "=) computation in " << Filename << " Line: " << Line
            << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName
            << ";\n     res overflows to-type max value: "
            << +ComputedOperationRes << " > "
            << +std::numeric_limits<LhsType>::max()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    Message << "unsafe conversion while (" << InnerOpName
            << "=) computation in " << Filename << " Line: " << Line
            << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName
            << ";\n     res overflows to-type min value: "
            << +ComputedOperationRes << " < "
            << +std::numeric_limits<LhsType>::lowest()
            << "; conversion does not safe the value\n"
            << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING, Message.str());
    break;
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    Message << "bool-conversion while (" << InnerOpName << "=) computation in "
            << Filename << " Line: " << Line
            << "\nlog: bool-conversions are not considered;\n     "
            << "lhs " << InnerOpName << "= rhs is computed as "
            << LhsComputationTypeName << " expression;\n     conversion of (("
            << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
            << " " << LhsInComputationType << ") " << InnerOpName << " " << +Rhs
            << ") = " << +ComputedOperationRes << " from "
            << LhsComputationTypeName << " to " << LhsTypeName << "\n";
    PUSH_WARNING(NOT_CONSIDERED_WARNING, Message.str());
    break;

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
}

} // namespace support

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpSum(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkSum<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " + " << +Rhs << " > "
            << +std::numeric_limits<LhsComputationType>::max()
            << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " + " << +Rhs << " < "
            << +std::numeric_limits<LhsComputationType>::lowest()
            << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  // after (LhsInComputationType + Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType + Rhs, "+", LhsTypeName,
      LhsComputationTypeName, Filename, Line);
  return Lhs += Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpDiff(LhsType& Lhs, RhsType Rhs,
                                const char* LhsTypeName,
                                const char* LhsComputationTypeName,
                                const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkDiff<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " - " << +Rhs << " > "
            << +std::numeric_limits<LhsComputationType>::max()
            << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " - " << +Rhs << " < "
            << +std::numeric_limits<LhsComputationType>::lowest()
            << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  // after (LhsInComputationType - Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType - Rhs, "-", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs -= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpMul(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkMul<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " * " << +Rhs << " > "
            << +std::numeric_limits<LhsComputationType>::max()
            << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " * " << +Rhs << " < "
            << +std::numeric_limits<LhsComputationType>::lowest()
            << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }

  // after (LhsInComputationType * Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType * Rhs, "*", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs *= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpDiv(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* Filename, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkDiv<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " / " << +Rhs << " > "
            << +std::numeric_limits<LhsComputationType>::max()
            << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::OVERFLOW_MIN:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " / " << +Rhs << " < "
            << +std::numeric_limits<LhsComputationType>::lowest()
            << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::DIV_BY_0:
    Message << LhsComputationTypeName << " /= by 0 in " << Filename
            << " Line: " << Line << "\nlog: lhs /= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_ERROR(DIVISION_BY_ZERO_ERROR, Message.str());

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiv");
  }

  // after (LhsInComputationType / Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType / Rhs, "/", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs /= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpMod(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkMod<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    Message << LhsComputationTypeName << " mod (%=) is undefined in "
            << Filename << " Line: " << Line
            << "\nlog: because division is undefined; overflow: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
            << ";\n     lhs %= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    PUSH_ERROR(UNDEFINED_MOD_ERROR, Message.str());

  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    Message << LhsComputationTypeName << " mod (%=) is undefined in "
            << Filename << " Line: " << Line
            << "\nlog: because division is undefined; overflow: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " / " << +Rhs << " < " << +std::numeric_limits<LhsType>::lowest()
            << ";\n     lhs %= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    PUSH_ERROR(UNDEFINED_MOD_ERROR, Message.str());

  case ArithmCheckRes::DIV_BY_0:
    Message << LhsComputationTypeName << " mod (%=) by 0 in " << Filename
            << " Line: " << Line << "\nlog: lhs %= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_ERROR(DIVISION_BY_ZERO_ERROR, Message.str());

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMod");
  }

  // after (LhsInComputationType % Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType % Rhs, "%", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs %= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftLeft(LhsType& Lhs, RhsType Rhs,
                                        const char* LhsTypeName,
                                        const char* LhsComputationTypeName,
                                        const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  using UnsignedLhsComputationType =
      typename std::make_unsigned<LhsComputationType>::type;
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkBitShiftLeft<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    Message << LhsComputationTypeName << " bitshift left (<<=) is undefined in "
            << Filename << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
            << " < 0;\n     lhs <<= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    Message << LhsComputationTypeName << " bitshift left (<<=) is undefined in "
            << Filename << " Line: " << Line
            << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
            << " >= " << +arithm_util::getTypeSizeInBits<LhsComputationType>()
            << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

#if __cplusplus > 201703L // only since C++20
  case ArithmCheckRes::OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: (" << +Lhs << " <<= " << +Rhs << ") > "
            << +std::numeric_limits<LhsType>::max() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING, Message.str());
    return Lhs <<= Rhs;

  case ArithmCheckRes::OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20:
    Message << LhsTypeName << " overflow in " << Filename << " Line: " << Line
            << "\nlog: (" << +Lhs << " <<= " << +Rhs << ") < "
            << +std::numeric_limits<LhsType>::min() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING, Message.str());
    return Lhs <<= Rhs;
#endif

  case ArithmCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    Message << LhsComputationTypeName << " bitshift left (<<=) is undefined in "
            << Filename << " Line: " << Line << "\nlog: negative lhs; "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " < 0;\n     lhs <<= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS:
    Message << LhsComputationTypeName << " bitshift left (<<=) is undefined in "
            << Filename << " Line: " << Line
            << "\nlog: signed lhs is non-negative, but result is not "
               "representable in unsigned version of lhs (computation) type; ("
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " << " << +Rhs << ") > "
            << std::numeric_limits<UnsignedLhsComputationType>::max()
            << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_LEFT_ERROR, Message.str());

  case ArithmCheckRes::OVERFLOW_MAX:
    Message << LhsComputationTypeName << " overflow in " << Filename
            << " Line: " << Line << "\nlog: "
            << "(" << LhsTypeName << " " << +Lhs << " -> "
            << LhsComputationTypeName << " " << +LhsInComputationType << ")"
            << " << " << +Rhs << " > "
            << +std::numeric_limits<LhsComputationType>::max()
            << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    OVERFLOW_DETECTED(LhsComputationType, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftLeft");
  }

  // after (LhsInComputationType << Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType << Rhs, "<<", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs <<= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftRight(LhsType& Lhs, RhsType Rhs,
                                         const char* LhsTypeName,
                                         const char* LhsComputationTypeName,
                                         const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);
  std::stringstream Message;

  switch (arithm::checkers::checkBitShiftRight<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    Message << LhsComputationTypeName
            << " bitshift right (>>=) is undefined in " << Filename
            << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
            << " < 0;\n     lhs >>= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_RIGHT_ERROR, Message.str());

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    Message << LhsComputationTypeName
            << " bitshift right (>>=) is undefined in " << Filename
            << " Line: " << Line << "\nlog: rhs >= number of bits in lhs type; "
            << +Rhs
            << " >= " << +arithm_util::getTypeSizeInBits<LhsComputationType>()
            << ";\n     lhs >>= rhs is computed as " << LhsComputationTypeName
            << " expression\n";
    PUSH_ERROR(UNDEFINED_BITSHIFT_RIGHT_ERROR, Message.str());

  case ArithmCheckRes::IMPL_DEFINED_OPERATION:
    Message << LhsTypeName
            << " bitshift right (>>=) is implementation-defined in " << Filename
            << " Line: " << Line << "\nlog: negative lhs; "
            << +LhsInComputationType
            << " < 0;\n     lhs >>= rhs is computed as "
            << LhsComputationTypeName << " expression\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING, Message.str());
    break;

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftRight");
  }

  // after (LhsInComputationType >> Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType >> Rhs, ">>", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs >>= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicAnd(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (&=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) & Rhs, "&", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs &= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicOr(LhsType& Lhs, RhsType Rhs,
                                   const char* LhsTypeName,
                                   const char* LhsComputationTypeName,
                                   const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (|=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) | Rhs, "|", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs |= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicXor(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* Filename, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (^=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) ^ Rhs, "^", LhsTypeName,
      LhsComputationTypeName, Filename, Line);

  return Lhs ^= Rhs;
}

} // namespace ub_tester::arithm::asserts

#undef OVERFLOW_DETECTED
#undef ARE_SAME_TYPES
#undef HAS_CONV_RANK_GEQ_THAN_INT
