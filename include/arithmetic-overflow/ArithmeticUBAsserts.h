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
  arithm_asserts::assert##Operation<LhsType, RhsType>((Lhs), (Rhs), #LhsType,  \
                                                      __FILE__, __LINE__)
#define ASSERT_UNOP(Operation, Expr, Type)                                     \
  arithm_asserts::assert##Operation<Type>((Expr), #Type, __FILE__, __LINE__)

#define ASSERT_COMPASSIGNOP(Operation, Lhs, Rhs, LhsType, LhsComputationType,  \
                            RhsType)                                           \
  arithm_asserts::assertCompAssignOp##Operation<LhsType, LhsComputationType,   \
                                                RhsType>(                      \
      (Lhs), (Rhs), #LhsType, #LhsComputationType, __FILE__, __LINE__)

#define OVERFLOW_ASSERT_FAILED(Type, DoIfWarning)                              \
  if (!std::numeric_limits<Type>::is_signed) {                                 \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE, DoIfWarning);                 \
  } else                                                                       \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

// in future failures and warnings will be collected by special class
#define ASSERT_FAILED(ExitCode)                                                \
  exit(arithm_asserts_exit_codes::ExitCode) // return 0 // for testing
#define PUSH_WARNING(WarningCode, DoIfWarning)                                 \
  std::cerr << "Warning " << arithm_asserts_exit_codes::WarningCode            \
            << " has been generated.\n";                                       \
  DoIfWarning;

#define ARE_SAME_TYPES(Type1, Type2) assert((std::is_same<Type1, Type2>::value))
#define HAS_CONV_RANK_GEQ_THAN_INT(Type)                                       \
  assert((arithm_ut::checkIfTypeHasConvRankGeqThanInt<Type>()));
// will be removed in future, when class for message-args appears
#define UNUSED_ASSERT_ARGS(Arg1, Arg2, Arg3, Arg4)                             \
  (void)Arg1;                                                                  \
  (void)Arg2;                                                                  \
  (void)Arg3;                                                                  \
  (void)Arg4;

namespace ub_tester {
namespace arithm_asserts {

// should be moved to others exit-codes
namespace arithm_asserts_exit_codes {

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

} // namespace arithm_asserts_exit_codes

using arithm_check::ArithmCheckRes;
using type_conv::TyCoCheckRes;

template <typename LhsType, typename RhsType>
LhsType assertSum(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm_check::checkSum<LhsType>(Lhs, Rhs)) {
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

  switch (arithm_check::checkDiff<LhsType>(Lhs, Rhs)) {
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

  switch (arithm_check::checkMul<LhsType>(Lhs, Rhs)) {
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
  switch (arithm_check::checkDiv<LhsType>(Lhs, Rhs)) {
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
  assert(std::numeric_limits<LhsType>::is_integer);
  ARE_SAME_TYPES(LhsType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected

  switch (arithm_check::checkMod<LhsType>(Lhs, Rhs)) {
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
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;

  switch (arithm_check::checkBitShiftLeft<LhsType, RhsType>(Lhs, Rhs)) {
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
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(LhsType); // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected

  switch (arithm_check::checkBitShiftRight<LhsType, RhsType>(Lhs, Rhs)) {
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

  switch (arithm_check::checkUnaryNeg<T>(Expr)) {
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

template <typename T>
T& assertPrefixIncr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (arithm_check::checkSum<T>(Expr, 1)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: ++(" << +Expr << ") > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return ++Expr);

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Prefix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    return ++Expr;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }
}
template <>
bool& assertPrefixIncr(bool& Expr, const char* TypeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool prefix increment is deprecated since C++17");
}

template <typename T>
T assertPostfixIncr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (arithm_check::checkSum<T>(Expr, 1)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Expr << ")++ > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return Expr++);

  case ArithmCheckRes::OVERFLOW_MIN:
    assert(0 && "Postfix increment assert detected OVERFLOW_MIN");

  case ArithmCheckRes::SAFE_OPERATION:
    return Expr++;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }
}
template <>
bool assertPostfixIncr(bool& Expr, const char* TypeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool postfix increment is deprecated since C++17");
}

template <typename T>
T& assertPrefixDecr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (arithm_check::checkDiff<T>(Expr, 1)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Prefix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: --(" << +Expr << ") < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return --Expr);

  case ArithmCheckRes::SAFE_OPERATION:
    return --Expr;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }
}
template <>
bool& assertPrefixDecr(bool& Expr, const char* TypeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool prefix decrement is deprecated since C++17");
}

template <typename T>
T assertPostfixDecr(T& Expr, const char* TypeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (arithm_check::checkDiff<T>(Expr, 1)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    assert(0 && "Postfix decrement assert detected OVERFLOW_MAX");

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << TypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Expr << ")-- < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, return Expr--);

  case ArithmCheckRes::SAFE_OPERATION:
    return Expr--;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }
}
template <>
bool assertPostfixDecr(bool& Expr, const char* TypeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(Expr, TypeName, FileName, Line);
  assert(0 && "bool postfix decrement is deprecated since C++17");
}

namespace arithm_asserts_support {

template <typename LhsType, typename LhsComputationType>
void checkCompAssignOpResTypeConv(LhsType Lhs, LhsComputationType Rhs,
                                  LhsComputationType ComputedOperationRes,
                                  const char* InnerOpName,
                                  const char* LhsTypeName,
                                  const char* LhsComputationTypeName,
                                  const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (
      type_conv::Conversions<LhsComputationType, LhsType>::checkIntegralConv(
          ComputedOperationRes)) {
  case TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expr;\n     conversion of (lhs "
              << InnerOpName << " rhs) = (" << +Lhs << " " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     negative value to unsigned type conversion does not "
                 "safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expr;\n     conversion of (lhs "
              << InnerOpName << " rhs) = (" << +Lhs << " " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     expr overflows to-type max value: "
              << +ComputedOperationRes << " > "
              << +std::numeric_limits<LhsType>::max()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN:
    std::cerr << "unsafe conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: lhs " << InnerOpName << "= rhs is computed as "
              << LhsComputationTypeName << " expr;\n     conversion of (lhs "
              << InnerOpName << " rhs) = (" << +Lhs << " " << InnerOpName << " "
              << +Rhs << ") = " << +ComputedOperationRes << " from "
              << LhsComputationTypeName << " to " << LhsTypeName
              << ";\n     expr overflows to-type min value: "
              << +ComputedOperationRes << " < "
              << +std::numeric_limits<LhsType>::lowest()
              << "; conversion does not safe the value\n";
    PUSH_WARNING(UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << InnerOpName << "=) computation in "
        << FileName << " Line: " << Line << "\nlog: lhs " << InnerOpName
        << "= rhs is computed as " << LhsComputationTypeName
        << " expr;\n     conversion of (lhs " << InnerOpName << " rhs) = ("
        << +Lhs << " " << InnerOpName << " " << +Rhs
        << ") = " << +ComputedOperationRes << " from " << LhsComputationTypeName
        << " to " << LhsTypeName
        << ";\n     expr overflows to-type max value: " << +ComputedOperationRes
        << " > " << +std::numeric_limits<LhsType>::max()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED:
    std::cerr
        << "unsafe conversion while (" << InnerOpName << "=) computation in "
        << FileName << " Line: " << Line << "\nlog: lhs " << InnerOpName
        << "= rhs is computed as " << LhsComputationTypeName
        << " expr;\n     conversion of (lhs " << InnerOpName << " rhs) = ("
        << +Lhs << " " << InnerOpName << " " << +Rhs
        << ") = " << +ComputedOperationRes << " from " << LhsComputationTypeName
        << " to " << LhsTypeName
        << ";\n     expr overflows to-type min value: " << +ComputedOperationRes
        << " < " << +std::numeric_limits<LhsType>::lowest()
        << "; conversion does not safe the value\n"
        << "     and is implementation defined because to-type is signed\n";
    PUSH_WARNING(IMPL_DEFINED_UNSAFE_CONV_WARNING_CODE, break);
  case TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED:
    std::cerr << "to-bool-conversion while (" << InnerOpName
              << "=) computation in " << FileName << " Line: " << Line
              << "\nlog: to-bool-conversions are not considered\n";

  case TyCoCheckRes::SAFE_CONVERSION:
    break;
  }
}

} // namespace arithm_asserts_support

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

  switch (arithm_check::checkSum<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " + " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " + " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs += rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkSum");
  }

  // after (LhsInComputationType + Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
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

  switch (arithm_check::checkDiff<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " - " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " - " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs -= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiff");
  }

  // after (LhsInComputationType - Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
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

  switch (arithm_check::checkMul<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " * " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " * " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs *= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }

  // after (LhsInComputationType * Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
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

  switch (arithm_check::checkDiv<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::OVERFLOW_MIN:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsComputationType>::lowest()
              << ";\n     lhs /= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsComputationTypeName << " /= by 0 in " << FileName
              << " Line: " << Line << "\nlog: lhs /= rhs is computed as "
              << LhsComputationTypeName << " expr\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkDiv");
  }

  // after (LhsInComputationType / Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, LhsInComputationType / Rhs, "/", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs /= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpMod(LhsType& Lhs, RhsType Rhs,
                               const char* LhsTypeName,
                               const char* LhsComputationTypeName,
                               const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm_check::checkMod<LhsComputationType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << LhsComputationTypeName << " mod (%=) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: because division is undefined; overflow: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
              << ";\n     lhs %= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
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
              << " expr\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case ArithmCheckRes::DIV_BY_0:
    std::cerr << LhsComputationTypeName << " mod (%=) by 0 in " << FileName
              << " Line: " << Line << "\nlog: lhs %= rhs is computed as "
              << LhsComputationTypeName << " expr\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMod");
  }

  // after (LhsInComputationType % Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, LhsInComputationType % Rhs, "%", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs %= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftLeft(LhsType& Lhs, RhsType Rhs,
                                        const char* LhsTypeName,
                                        const char* LhsComputationTypeName,
                                        const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  typedef typename std::make_unsigned<LhsComputationType>::type
      UnsignedLhsComputationType;
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm_check::checkBitShiftLeft<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsComputationTypeName
              << " bitshift left (<<=) is undefined in " << FileName
              << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
              << " < 0;\n     lhs <<= rhs is computed as "
              << LhsComputationTypeName << " expr\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsComputationTypeName
              << " bitshift left (<<=) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsComputationType>()
              << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
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
              << LhsComputationTypeName << " expr\n";
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
        << " expr\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);

  case ArithmCheckRes::OVERFLOW_MAX:
    std::cerr << LhsComputationTypeName << " overflow in " << FileName
              << " Line: " << Line << "\nlog: "
              << "(" << LhsTypeName << " " << +Lhs << " -> "
              << LhsComputationTypeName << " " << +LhsInComputationType << ")"
              << " << " << +Rhs << " > "
              << +std::numeric_limits<LhsComputationType>::max()
              << ";\n     lhs <<= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    OVERFLOW_ASSERT_FAILED(LhsComputationType, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftLeft");
  }

  // after (LhsInComputationType << Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, LhsInComputationType << Rhs, "<<", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs <<= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpBitShiftRight(LhsType& Lhs, RhsType Rhs,
                                         const char* LhsTypeName,
                                         const char* LhsComputationTypeName,
                                         const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType);             // integral promotion is expected
  HAS_CONV_RANK_GEQ_THAN_INT(RhsType); // integral promotion is expected
  LhsComputationType LhsInComputationType =
      static_cast<LhsComputationType>(Lhs);

  switch (arithm_check::checkBitShiftRight<LhsComputationType, RhsType>(
      static_cast<LhsComputationType>(Lhs), Rhs)) {
  case ArithmCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsComputationTypeName
              << " bitshift right (>>=) is undefined in " << FileName
              << " Line: " << Line << "\nlog: negative rhs; " << +Rhs
              << " < 0;\n     lhs >>= rhs is computed as "
              << LhsComputationTypeName << " expr\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsComputationTypeName
              << " bitshift right (>>=) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: rhs >= number of bits in lhs type; " << +Rhs
              << " >= " << +arithm_ut::getTypeSizeInBits<LhsComputationType>()
              << ";\n     lhs >>= rhs is computed as " << LhsComputationTypeName
              << " expr\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);

  case ArithmCheckRes::IMPL_DEFINED_OPERATION:
    std::cerr << LhsTypeName
              << " bitshift right (>>=) is implementation-defined in "
              << FileName << " Line: " << Line << "\nlog: negative lhs; "
              << +LhsInComputationType
              << " < 0;\n     lhs >>= rhs is computed as "
              << LhsComputationTypeName << " expr\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING_CODE, break);

  case ArithmCheckRes::SAFE_OPERATION:
    break;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkBitShiftRight");
  }

  // after (LhsInComputationType >> Rhs) is computed, it is converted to LhsType
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, LhsInComputationType >> Rhs, ">>", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs >>= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicAnd(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (&=) cannot overflow or cause UB, so only conversion check is needed
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) & Rhs, "&", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs &= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicOr(LhsType& Lhs, RhsType Rhs,
                                   const char* LhsTypeName,
                                   const char* LhsComputationTypeName,
                                   const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (|=) cannot overflow or cause UB, so only conversion check is needed
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) | Rhs, "|", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs |= Rhs;
}

template <typename LhsType, typename LhsComputationType, typename RhsType>
LhsType& assertCompAssignOpLogicXor(LhsType& Lhs, RhsType Rhs,
                                    const char* LhsTypeName,
                                    const char* LhsComputationTypeName,
                                    const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<LhsComputationType>::is_integer);
  ARE_SAME_TYPES(LhsComputationType, RhsType);
  HAS_CONV_RANK_GEQ_THAN_INT(
      LhsComputationType); // integral promotion is expected

  // (^=) cannot overflow or cause UB, so only conversion check is needed
  arithm_asserts_support::checkCompAssignOpResTypeConv<LhsType,
                                                       LhsComputationType>(
      Lhs, Rhs, static_cast<LhsComputationType>(Lhs) ^ Rhs, "^", LhsTypeName,
      LhsComputationTypeName, FileName, Line);

  return Lhs ^= Rhs;
}

} // namespace arithm_asserts
} // namespace ub_tester
