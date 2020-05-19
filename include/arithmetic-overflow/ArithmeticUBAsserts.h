#pragma once

#include "ArithmeticUBCheckers.h"
#include "TypeConversionCheckers.h"
#include <cstring>
#include <iostream>

// Lhs and Rhs can be put in Assert-functions as strings to improve error-log
/* Binary operations usually have equal Lhs and Rhs types, but there are some
 * exceptions like bitshift operators (can have different integer types).
 * However, return type of binary operator is always Lhs type. */
#define ASSERT_BINOP(Operation, Lhs, Rhs, LhsType, RhsType)                    \
  arithm::asserts::assert##Operation<LhsType, RhsType>((Lhs), (Rhs), #LhsType, \
                                                       __FILE__, __LINE__)
#define ASSERT_UNOP(Operation, Expr, Type)                                     \
  arithm::asserts::assert##Operation<Type>((Expr), #Type, __FILE__, __LINE__)

#define ASSERT_COMPASSIGNOP(Operation, Lhs, Rhs, LhsType, LhsComputationType,  \
                            RhsType)                                           \
  arithm::asserts::assertCompAssignOp##Operation<LhsType, LhsComputationType,  \
                                                 RhsType>(                     \
      (Lhs), (Rhs), #LhsType, #LhsComputationType, __FILE__, __LINE__)

#define IMPLICIT_CAST(SubExpr, FromType, ToType)                               \
  arithm::asserts::casts::assertIntegralCast<FromType, ToType>(                \
      (SubExpr), #FromType, #ToType, __FILE__, __LINE__)

#define OVERFLOW_ASSERT_FAILED(Type, DoIfWarning)                              \
  if (!std::numeric_limits<Type>::is_signed) {                                 \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE, DoIfWarning);                 \
  } else                                                                       \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

// in future failures and warnings will be collected by special class
#define ASSERT_FAILED(ExitCode)                                                \
  exit(arithm::exit_codes::ExitCode) // return 0 // for testing
#define PUSH_WARNING(WarningCode, DoIfWarning)                                 \
  std::cerr << "Warning " << arithm::exit_codes::WarningCode                   \
            << " has been generated.\n";                                       \
  DoIfWarning;

#define ARE_SAME_TYPES(Type1, Type2)                                           \
  static_assert(std::is_same<Type1, Type2>::value)
#define HAS_CONV_RANK_GEQ_THAN_INT(Type)                                       \
  static_assert(arithm_ut::checkIfTypeHasConvRankGeqThanInt<Type>());
// will be removed in future, when class for message-args appears
#define UNUSED_ASSERT_ARGS(Arg1, Arg2, Arg3, Arg4)                             \
  (void)Arg1;                                                                  \
  (void)Arg2;                                                                  \
  (void)Arg3;                                                                  \
  (void)Arg4;

namespace ub_tester {
namespace arithm::exit_codes {

constexpr int OVERFLOW_EXIT_CODE = -1;
constexpr int DIVISION_BY_ZERO_EXIT_CODE = -2;
constexpr int UNDEFINED_MOD_EXIT_CODE = -3;
constexpr int UNDEFINED_BITSHIFT_LEFT_EXIT_CODE = -4;
constexpr int UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE = -5;

constexpr int UNSIGNED_OVERFLOW_WARNING_CODE = -6;
constexpr int OVERFLOW_IN_BITSHIFT_CXX20_WARNING_CODE = -7;
constexpr int IMPL_DEFINED_WARNING_CODE = -8;
constexpr int UNSAFE_CONV_WARNING_CODE = -9;
constexpr int IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE = -10;
constexpr int NOT_CONSIDERED_WARNING_CODE = -11;

} // namespace arithm::exit_codes

namespace arithm::asserts {

using arithm::checkers::ArithmCheckRes;
using type_conv::TyCoCheckRes;

namespace casts {

template <typename FromType, typename ToType>
ToType assertIntegralCast(FromType SubExpr, const char* FromTypeName,
                          const char* ToTypeName, const char* FileName,
                          int Line) {
  static_assert(std::numeric_limits<FromType>::is_integer);
  static_assert(std::numeric_limits<ToType>::is_integer);
  ToType SubExprInToType = static_cast<ToType>(SubExpr);

  switch (
      type_conv::Conversions<FromType, ToType>::checkIntegralConv(SubExpr)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    std::cerr << "unsafe conversion while implicit cast in " << FileName
              << " Line: " << Line << "\nlog: "
              << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
              << ToTypeName << " " << +SubExprInToType << ") from "
              << FromTypeName << " to " << ToTypeName
              << ";\n     negative value to unsigned type conversion does not "
                 "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    std::cerr << "unsafe conversion while implicit cast in " << FileName
              << " Line: " << Line << "\nlog: "
              << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
              << ToTypeName << " " << +SubExprInToType << ") from "
              << FromTypeName << " to " << ToTypeName
              << ";\n     res overflows to-type max value: " << +SubExpr
              << " > " << +std::numeric_limits<ToType>::max()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    std::cerr << "unsafe conversion while implicit cast in " << FileName
              << " Line: " << Line << "\nlog: "
              << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
              << ToTypeName << " " << +SubExprInToType << ") from "
              << FromTypeName << " to " << ToTypeName
              << ";\n     res overflows to-type min value: " << +SubExpr
              << " < " << +std::numeric_limits<ToType>::lowest()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while implicit cast in " << FileName
        << " Line: " << Line << "\nlog: "
        << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
        << ToTypeName << " " << +SubExprInToType << ") from " << FromTypeName
        << " to " << ToTypeName
        << ";\n     res overflows to-type max value: " << +SubExpr << " > "
        << +std::numeric_limits<ToType>::max()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while implicit cast in " << FileName
        << " Line: " << Line << "\nlog: "
        << "conversion of (" << FromTypeName << " " << +SubExpr << " -> "
        << ToTypeName << " " << +SubExprInToType << ") from " << FromTypeName
        << " to " << ToTypeName
        << ";\n     res overflows to-type min value: " << +SubExpr << " < "
        << +std::numeric_limits<ToType>::lowest()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    std::cerr << "to-bool-conversion while implicit cast in " << FileName
              << " Line: " << Line
              << "\nlog: to-bool-conversions are not considered\n";
    PUSH_WARNING(NOT_CONSIDERED_WARNING_CODE, break);

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
  return SubExpr;
}

} // namespace casts

template <typename LhsType, typename RhsType>
LhsType assertSum(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm::checkers::checkSum<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " + " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs + Rhs);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " + " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs + Rhs);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs + Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiff(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                   const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm::checkers::checkDiff<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " - " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs - Rhs);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " - " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs - Rhs);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs - Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMul(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm::checkers::checkMul<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " * " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs * Rhs);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " * " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs * Rhs);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs * Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiv(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  // check for flt-point in future: minLim <= (Lhs / 0-approx Rhs) <= maxLim
  switch (arithm::checkers::checkDiv<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " / " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs / Rhs);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs / Rhs);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsTypeName << " division by 0 in " << FileName
              << " Line: " << Line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs / Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiv");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMod(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm::checkers::checkMod<LhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << LhsTypeName << " mod (%) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: because division is undefined; overflow: " << +Lhs
              << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
              << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    std::cerr << LhsTypeName << " mod (%) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: because division is undefined; overflow: " << +Lhs
              << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsTypeName << " mod (%) by 0 in " << FileName
              << " Line: " << Line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs % Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMod");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftLeft(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                           const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;

  switch (arithm::checkers::checkBitShiftLeft<LhsType, RhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative rhs; "
              << +Rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsType>() << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

#if __cplusplus > 201703L // only since C++20
  case ArithmCheckRes::OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " << " << +Rhs << ") > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING_CODE, return Lhs << Rhs);

  case ArithmCheckRes::OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " << " << +Rhs << ") < "
              << +std::numeric_limits<LhsType>::min() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING_CODE, return Lhs << Rhs);
#endif

  case ArithmCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative lhs; "
              << +Lhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: signed lhs is non-negative, but result is not "
                 "representable in unsigned version of lhs type; ("
              << +Lhs << " << " << +Rhs << ") > "
              << std::numeric_limits<UnsignedLhsType>::max() << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " << " << +Rhs << ") > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, return Lhs << Rhs);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs << Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftLeft");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftRight(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                            const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected

  switch (arithm::checkers::checkBitShiftRight<LhsType, RhsType>(Lhs, Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsTypeName << " bitshift right (>>) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative rhs; "
              << +Rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsTypeName << " bitshift right (>>) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsType>() << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::IMPL_DEFINED_OPERATION:
    std::cerr << LhsTypeName
              << " bitshift right (>>) is implementation-defined in "
              << FileName << " Line: " << Line << "\nlog: negative lhs; "
              << +Lhs << " < 0\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING_CODE, return Lhs >> Rhs);

  case ArithmCheckRes::SAFE_OPERATION:
    return Lhs >> Rhs;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftRight");
  }
}

template <typename T>
T assertUnaryNeg(T Expr, const char* TypeName, const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  HAS_CONV_RANK_GEQ_THAN_INT(T); // integral promotion is expected

  switch (arithm::checkers::checkUnaryNeg<T>(Expr)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: -(" << +Expr << ") > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return -Expr);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: -(" << +Expr << ") < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return -Expr);

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
                                   const char* FileName, int Line) {
  static_assert(std::numeric_limits<Type>::is_integer);
  static_assert(std::numeric_limits<CommonType>::is_integer);
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);

  switch (type_conv::Conversions<CommonType, Type>::checkIntegralConv(
      ComputedOperationRes)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    std::cerr << "unsafe conversion while (" << OpName << ") computation in "
              << FileName << " Line: " << Line
              << "\nlog: " << OpAppliedOnExprName << " is computed as "
              << CompAssignOpAppliedOnExprName << ", i.e. as " << CommonTypeName
              << " expression;\n     conversion of ((" << TypeName << " "
              << +Expr << " -> " << CommonTypeName << " " << +ExprInCommonType
              << ") " << InnerOpName << " " << 1
              << ") = " << +ComputedOperationRes << " from " << CommonTypeName
              << " to " << TypeName
              << ";\n     negative value to unsigned type conversion does not "
                 "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    std::cerr << "unsafe conversion while (" << OpName << ") computation in "
              << FileName << " Line: " << Line
              << "\nlog: " << OpAppliedOnExprName << " is computed as "
              << CompAssignOpAppliedOnExprName << ", i.e. as " << CommonTypeName
              << " expression;\n     conversion of ((" << TypeName << " "
              << +Expr << " -> " << CommonTypeName << " " << +ExprInCommonType
              << ") " << InnerOpName << " " << 1
              << ") = " << +ComputedOperationRes << " from " << CommonTypeName
              << " to " << TypeName
              << ";\n     res overflows to-type max value: "
              << +ComputedOperationRes << " > "
              << +std::numeric_limits<Type>::max()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    std::cerr << "unsafe conversion while (" << OpName << ") computation in "
              << FileName << " Line: " << Line
              << "\nlog: " << OpAppliedOnExprName << " is computed as "
              << CompAssignOpAppliedOnExprName << ", i.e. as " << CommonTypeName
              << " expression;\n     conversion of ((" << TypeName << " "
              << +Expr << " -> " << CommonTypeName << " " << +ExprInCommonType
              << ") " << InnerOpName << " " << 1
              << ") = " << +ComputedOperationRes << " from " << CommonTypeName
              << " to " << TypeName
              << ";\n     res overflows to-type min value: "
              << +ComputedOperationRes << " < "
              << +std::numeric_limits<Type>::lowest()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << OpName << ") computation in "
        << FileName << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
        << " is computed as " << CompAssignOpAppliedOnExprName << ", i.e. as "
        << CommonTypeName << " expression;\n     conversion of ((" << TypeName
        << " " << +Expr << " -> " << CommonTypeName << " " << +ExprInCommonType
        << ") " << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
        << " from " << CommonTypeName << " to " << TypeName
        << ";\n     res overflows to-type max value: " << +ComputedOperationRes
        << " > " << +std::numeric_limits<Type>::max()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << OpName << ") computation in "
        << FileName << " Line: " << Line << "\nlog: " << OpAppliedOnExprName
        << " is computed as " << CompAssignOpAppliedOnExprName << ", i.e. as "
        << CommonTypeName << " expression;\n     conversion of ((" << TypeName
        << " " << +Expr << " -> " << CommonTypeName << " " << +ExprInCommonType
        << ") " << InnerOpName << " " << 1 << ") = " << +ComputedOperationRes
        << " from " << CommonTypeName << " to " << TypeName
        << ";\n     res overflows to-type min value: " << +ComputedOperationRes
        << " < " << +std::numeric_limits<Type>::lowest()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    std::cerr << "to-bool-conversion while (" << OpName << ") computation in "
              << FileName << " Line: " << Line
              << "\nlog: to-bool-conversions are not considered\n";
    PUSH_WARNING(NOT_CONSIDERED_WARNING_CODE, break);

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
T& assertPrefixIncr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  typedef typename std::common_type<T, int>::type CommonType;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_ut::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkSum<CommonType>(ExprInCommonType,
                                                 RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << CommonTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: ++(" << TypeName << " " << +Expr
              << " -> " << CommonTypeName << " " << +ExprInCommonType << ") > "
              << +std::numeric_limits<T>::max()
              << "\n     ++expr is computed as expr += 1, i.e. as "
              << CommonTypeName << " expression\n";
    OVERFLOW_ASSERT_FAILED(CommonType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Prefix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType + 1, "prefix ++", "+", "++expr", "expr += 1",
      TypeName, CommonTypeName.c_str(), FileName, Line);

  return ++Expr;
}
template <>
inline bool& assertPrefixIncr<bool>(bool& Expr, const char* TypeName,
                                    const char* FileName, int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool prefix increment is deprecated since C++17");
}

template <typename T>
T assertPostfixIncr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  typedef typename std::common_type<T, int>::type CommonType;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_ut::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkSum<CommonType>(ExprInCommonType,
                                                 RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << CommonTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: (" << TypeName << " " << +Expr
              << " -> " << CommonTypeName << " " << +ExprInCommonType
              << ")++ > " << +std::numeric_limits<T>::max()
              << "\n     expr++ is computed as expr += 1, i.e. as "
              << CommonTypeName << " expression\n";
    OVERFLOW_ASSERT_FAILED(CommonType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Postfix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType + 1, "postfix ++", "+", "expr++", "expr += 1",
      TypeName, CommonTypeName.c_str(), FileName, Line);

  return Expr++;
}
template <>
inline bool assertPostfixIncr<bool>(bool& Expr, const char* TypeName,
                                    const char* FileName, int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool postfix increment is deprecated since C++17");
}

template <typename T>
T& assertPrefixDecr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  typedef typename std::common_type<T, int>::type CommonType;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_ut::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkDiff<CommonType>(ExprInCommonType,
                                                  RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Prefix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << CommonTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: --(" << TypeName << " " << +Expr
              << " -> " << CommonTypeName << " " << +ExprInCommonType << ") < "
              << +std::numeric_limits<T>::lowest()
              << "\n     --expr is computed as expr -= 1, i.e. as "
              << CommonTypeName << " expression\n";
    OVERFLOW_ASSERT_FAILED(CommonType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType - 1, "prefix --", "-", "--expr", "expr -= 1",
      TypeName, CommonTypeName.c_str(), FileName, Line);

  return --Expr;
}
template <>
inline bool& assertPrefixDecr<bool>(bool& Expr, const char* TypeName,
                                    const char* FileName, int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool prefix decrement is deprecated since C++17");
}

template <typename T>
T assertPostfixDecr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);
  typedef typename std::common_type<T, int>::type CommonType;
  HAS_CONV_RANK_GEQ_THAN_INT(CommonType);
  assert((type_conv::Conversions<T, CommonType>::checkIntegralConv(Expr) ==
          TyCoCheckRes::SAFE_CONVERSION));
  assert((type_conv::Conversions<int, CommonType>::checkIntegralConv(1) ==
          TyCoCheckRes::SAFE_CONVERSION));
  CommonType ExprInCommonType = static_cast<CommonType>(Expr);
  CommonType RhsInCommonType = static_cast<CommonType>(1);

  // temporary solution of getting CommonTypeName as string
  std::string CommonTypeName =
      arithm_ut::tmp_functions::getIntTypeAsString<CommonType>();
  if (CommonTypeName == "undefined type")
    CommonTypeName = "std::common_type<" + std::string(TypeName) + ", int>";

  switch (arithm::checkers::checkDiff<CommonType>(ExprInCommonType,
                                                  RhsInCommonType)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Postfix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << CommonTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: (" << TypeName << " " << +Expr
              << " -> " << CommonTypeName << " " << +ExprInCommonType
              << ")-- < " << +std::numeric_limits<T>::lowest()
              << "\n     expr-- is computed as expr -= 1, i.e. as "
              << CommonTypeName << " expression\n";
    OVERFLOW_ASSERT_FAILED(CommonType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  support::assertIncrOrDecrOpResTypeConv<T, CommonType>(
      Expr, ExprInCommonType - 1, "postfix --", "-", "expr--", "expr -= 1",
      TypeName, CommonTypeName.c_str(), FileName, Line);

  return Expr--;
}
template <>
inline bool assertPostfixDecr<bool>(bool& Expr, const char* TypeName,
                                    const char* FileName, int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool postfix decrement is deprecated since C++17");
}

namespace support {

template <typename LhsType, typename LhsComputationType>
void checkCompAssignOpResTypeConv(LhsType Lhs, LhsComputationType Rhs,
                                  LhsComputationType ComputedOperationRes,
                                  const char* InnerOpName,
                                  const char* LhsTypeName,
                                  const char* LhsComputationTypeName,
                                  const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (
      type_conv::Conversions<LhsComputationType, LhsType>::checkIntegralConv(
          ComputedOperationRes)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expression;\n     conversion of (("
              << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
              << " " << LhsInComputationType << ") " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     negative value to unsigned type conversion does not "
                 "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expression;\n     conversion of (("
              << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
              << " " << LhsInComputationType << ") " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     res overflows to-type max value: "
              << +ComputedOperationRes << " > "
              << +std::numeric_limits<LhsType>::max()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expression;\n     conversion of (("
              << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
              << " " << LhsInComputationType << ") " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     res overflows to-type min value: "
              << +ComputedOperationRes << " < "
              << +std::numeric_limits<LhsType>::lowest()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << InnerOpName << "=) computation in "
        << FileName << " Line: " << Line << "\nlog: lhs " << InnerOpName
        << "= rhs is computed as " << LhsComputationTypeName
        << " expression;\n     conversion of ((" << LhsTypeName << " " << +Lhs
        << " -> " << LhsComputationTypeName << " " << LhsInComputationType
        << ") " << InnerOpName << " " << +Rhs << ") = " << +ComputedOperationRes
        << " from " << LhsComputationTypeName << " to " << LhsTypeName
        << ";\n     res overflows to-type max value: " << +ComputedOperationRes
        << " > " << +std::numeric_limits<LhsType>::max()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << InnerOpName << "=) computation in "
        << FileName << " Line: " << Line << "\nlog: lhs " << InnerOpName
        << "= rhs is computed as " << LhsComputationTypeName
        << " expression;\n     conversion of ((" << LhsTypeName << " " << +Lhs
        << " -> " << LhsComputationTypeName << " " << LhsInComputationType
        << ") " << InnerOpName << " " << +Rhs << ") = " << +ComputedOperationRes
        << " from " << LhsComputationTypeName << " to " << LhsTypeName
        << ";\n     res overflows to-type min value: " << +ComputedOperationRes
        << " < " << +std::numeric_limits<LhsType>::lowest()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    std::cerr << "to-bool-conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: to-bool-conversions are not considered\n";
    PUSH_WARNING(NOT_CONSIDERED_WARNING_CODE, break);

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
}

} // namespace support

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpSum(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkSum<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " + " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " + " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  // after (LhsInComputationType + Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType + Rhs, "+", LhsTypeName,
      LhsComputationTypeName, FileName, Line);
  return Lhs += Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpDiff(LhsType& Lhs, RhsType Rhs,
                                const char* LhsTypeName,
                                const char* LhsComputationTypeName,
                                const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkDiff<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " - " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " - " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  // after (LhsInComputationType - Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType - Rhs, "-", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs -= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpMul(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkMul<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " * " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " * " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }

  // after (LhsInComputationType * Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType * Rhs, "*", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs *= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpDiv(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  FLT_POINT_NOT_SUPPORTED(LhsComputationType);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkDiv<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsComputationTypeName << " /= by 0 in " << FileName
              << " Line: " << Line << "\nlog: lhs /= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiv");
  }

  // after (LhsInComputationType / Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType / Rhs, "/", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs /= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpMod(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkMod<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << LhsComputationTypeName << " mod (%=) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: because division is undefined; overflow: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
              << ";\n     lhs %= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    std::cerr << LhsComputationTypeName << " mod (%=) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: because division is undefined; overflow: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest()
              << ";\n     lhs %= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsComputationTypeName << " mod (%=) by 0 in " << FileName
              << " Line: " << Line << "\nlog: lhs %= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMod");
  }

  // after (LhsInComputationType % Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType % Rhs, "%", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs %= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftLeft(LhsType& Lhs, RhsType Rhs,
                                        const char* LhsTypeName,
                                        const char* LhsComputationTypeName,
                                        const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  typedef typename std::make_unsigned<LhsComputationType>::type
      UnsignedLhsComputationType;
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkBitShiftLeft<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsComputationTypeName
              << " bitshift left (<<=) is undefined in " << FileName
              << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
              << " < 0;\n     lhs <<= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsComputationTypeName
              << " bitshift left (<<=) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsComputationType>()
              << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

#if __cplusplus > 201703L // only since C++20
  case ArithmCheckRes::OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " <<= " << +Rhs << ") > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING_CODE, return Lhs <<= Rhs);

  case ArithmCheckRes::OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " <<= " << +Rhs << ") < "
              << +std::numeric_limits<LhsType>::min() << "\n";
    PUSH_WARNING(OVERFLOW_IN_BITSHIFT_CXX20_WARNING_CODE, return Lhs <<= Rhs);
#endif

  case ArithmCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    std::cerr << LhsComputationTypeName
              << " bitshift left (<<=) is undefined in " << FileName
              << " Line: " << Line << "\nlog: negative lhs; "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " < 0;\n     lhs <<= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS:
    std::cerr
        << LhsComputationTypeName << " bitshift left (<<=) is undefined in "
        << FileName << " Line: " << Line
        << "\nlog: signed lhs is non-negative, but result is not "
           "representable in unsigned version of lhs (computation) type; ("
        << "(" << LhsTypeName << " " << +Lhs << " -> " << LhsComputationTypeName
        << " " << +LhsInComputationType << ")"
        << " << " << +Rhs << ") > "
        << std::numeric_limits<UnsignedLhsComputationType>::max()
        << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
        << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " << " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftLeft");
  }

  // after (LhsInComputationType << Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType << Rhs, "<<", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs <<= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftRight(LhsType& Lhs, RhsType Rhs,
                                         const char* LhsTypeName,
                                         const char* LhsComputationTypeName,
                                         const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm::checkers::checkBitShiftRight<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsComputationTypeName
              << " bitshift right (>>=) is undefined in " << FileName
              << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
              << " < 0;\n     lhs >>= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsComputationTypeName
              << " bitshift right (>>=) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsComputationType>()
              << ";\n     lhs >>= rhs is computed as " << LhsComputationTypeName
              << " expression\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::IMPL_DEFINED_OPERATION:
    std::cerr << LhsTypeName
              << " bitshift right (>>=) is implementation-defined in "
              << FileName << " Line: " << Line << "\nlog: negative lhs; "
              << +LhsInComputationType
              << " < 0;\n     lhs >>= rhs is computed as "
              << LhsComputationTypeName << " expression\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING_CODE, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftRight");
  }

  // after (LhsInComputationType >> Rhs) is computed, it is converted to
  // LhsType
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, LhsInComputationType >> Rhs, ">>", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs >>= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicAnd(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (&=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) & Rhs, "&", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs &= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicOr(LhsType& Lhs, RhsType Rhs,
                                   const char* LhsTypeName,
                                   const char* LhsComputationTypeName,
                                   const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (|=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) | Rhs, "|", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs |= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicXor(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* FileName, int Line) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (^=) cannot overflow or cause UB, so only conversion check is needed
  support::checkCompAssignOpResTypeConv<LhsType, LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) ^ Rhs, "^", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs ^= Rhs;
}

} // namespace arithm::asserts
} // namespace ub_tester
