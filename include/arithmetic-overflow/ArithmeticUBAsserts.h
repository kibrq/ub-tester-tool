#pragma once

#include "ArithmeticUBCheckers.h"
#include <cstring>
#include <iostream>

// Lhs and Rhs can be put in Assert-functions as strings to improve error-log
/* Binary operations usually have equal Lhs and Rhs types, but there are some
 * exceptions like bitshift operators (can have different integer types).
 * But return type of binary operator is always Lhs type. */
#define ASSERT_BINOP(Operation, Lhs, Rhs, LhsType, RhsType)                    \
  assert##Operation<LhsType, RhsType>((Lhs), (Rhs), #LhsType, __FILE__,        \
                                      __LINE__)
#define ASSERT_UNOP(Operation, Expr, Type)                                     \
  assert##Operation<Type>((Expr), #Type, __FILE__, __LINE__)

#define OVERFLOW_ASSERT_FAILED(Type, ReturnExprIfWarning)                      \
  if (!std::numeric_limits<Type>::is_signed) {                                 \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE, ReturnExprIfWarning);         \
  } else                                                                       \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

// in future failures and warnings will be collected by special class
#define ASSERT_FAILED(ExitCode) exit(ExitCode) // return 0 // for testing
#define PUSH_WARNING(WarningCode, ReturnExprIfWarning)                         \
  std::cerr << "Warning " << WarningCode << " has been generated.\n";          \
  return ReturnExprIfWarning;

#define ARE_SAME_TYPES(Type1, Type2)                                           \
  typedef std::is_same<Type1, Type2> AreSameTypes__;                           \
  assert(AreSameTypes__::value)
// will be removed in future, when class for message-args appears
#define UNUSED_ASSERT_ARGS(Arg1, Arg2, Arg3, Arg4)                             \
  (void)Arg1;                                                                  \
  (void)Arg2;                                                                  \
  (void)Arg3;                                                                  \
  (void)Arg4;

namespace ub_tester {

// should be moved to others exit-codes
constexpr int OVERFLOW_EXIT_CODE = -1;
constexpr int DIVISION_BY_ZERO_EXIT_CODE = -2;
constexpr int UNDEFINED_MOD_EXIT_CODE = -3;
constexpr int UNDEFINED_BITSHIFT_LEFT_EXIT_CODE = -4;
constexpr int UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE = -5;

constexpr int UNSIGNED_OVERFLOW_WARNING_CODE = -6;
constexpr int IMPL_DEFINED_WARNING_CODE = -7;

template <typename LhsType, typename RhsType>
LhsType assertSum(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckSum<LhsType>(Lhs, Rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " + " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs + Rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " + " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs + Rhs);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs + Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckSum");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiff(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                   const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckDiff<LhsType>(Lhs, Rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " - " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs - Rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " - " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs - Rhs);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs - Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiff");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMul(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckMul<LhsType>(Lhs, Rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " * " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs * Rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " * " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs * Rhs);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs * Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckMul");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertDiv(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  // check for flt-point in future: minLim <= (Lhs / 0-approx Rhs) <= maxLim
  switch (UBCheckDiv<LhsType>(Lhs, Rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " / " << +Rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs / Rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: " << +Lhs << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs / Rhs);

  case UBCheckRes::DIV_BY_0:
    std::cerr << LhsTypeName << " division by 0 in " << FileName
              << " Line: " << Line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs / Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiv");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertMod(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                  const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckMod<LhsType>(Lhs, Rhs)) {
  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << LhsTypeName << " mod (%) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: because division is undefined; overflow: " << +Lhs
              << " / " << +Rhs << " > " << +std::numeric_limits<LhsType>::max()
              << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    std::cerr << LhsTypeName << " mod (%) is undefined in " << FileName
              << " Line: " << Line
              << "\nlog: because division is undefined; overflow: " << +Lhs
              << " / " << +Rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case UBCheckRes::DIV_BY_0:
    std::cerr << LhsTypeName << " mod (%) by 0 in " << FileName
              << " Line: " << Line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs % Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckMod");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftLeft(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                           const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;

  switch (UBCheckBitShiftLeft<LhsType, RhsType>(Lhs, Rhs)) {
  case UBCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative Rhs; "
              << +Rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: Rhs >= number of bits in Lhs type; " << +Rhs
              << " >= " << +(sizeof(LhsType) * CHAR_BIT) << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    std::cerr << LhsTypeName << " bitshift left (<<) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative Lhs; "
              << +Lhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS:
    std::cerr << LhsTypeName << " bitshift left (<<) undefined is in "
              << FileName << " Line: " << Line
              << "\nlog: signed Lhs is non-negative, but result is not "
                 "representable in unsigned version of LhsType; ("
              << +Lhs << " << " << +Rhs << ") > "
              << std::numeric_limits<UnsignedLhsType>::max() << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << LhsTypeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +Lhs << " << " << +Rhs << ") > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, Lhs << Rhs);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs << Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckBitShiftLeft");
  }
}

template <typename LhsType, typename RhsType>
LhsType assertBitShiftRight(LhsType Lhs, RhsType Rhs, const char* LhsTypeName,
                            const char* FileName, int Line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);

  switch (UBCheckBitShiftRight<LhsType, RhsType>(Lhs, Rhs)) {
  case UBCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << LhsTypeName << " bitshift right (>>) is undefined in "
              << FileName << " Line: " << Line << "\nlog: negative Rhs; "
              << +Rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << LhsTypeName << " bitshift right (>>) is undefined in "
              << FileName << " Line: " << Line
              << "\nlog: Rhs >= number of bits in Lhs type; " << +Rhs
              << " >= " << +(sizeof(LhsType) * CHAR_BIT) << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);
  case UBCheckRes::IMPL_DEFINED_OPERATION:
    std::cerr << LhsTypeName
              << " bitshift right (>>) is implementation-defined in "
              << FileName << " Line: " << Line << "\nlog: negative Lhs; "
              << +Lhs << " < 0\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING_CODE, Lhs >> Rhs);

  case UBCheckRes::SAFE_OPERATION:
    return Lhs >> Rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckBitShiftRight");
  }
}

template <typename T>
T assertUnaryNeg(T expr, const char* typeName, const char* FileName, int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckUnaryNeg<T>(expr)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: -(" << +expr << ") > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, -expr);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: -(" << +expr << ") < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, -expr);

  case UBCheckRes::SAFE_OPERATION:
    return -expr;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckUnaryNeg");
  }
}

template <typename T>
T& assertPrefixIncr(T& expr, const char* typeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckSum<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: ++(" << +expr << ") > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, ++expr);

  case UBCheckRes::OVERFLOW_MIN:
    assert(0 && "Prefix increment assert detected OVERFLOW_MIN");

  case UBCheckRes::SAFE_OPERATION:
    return ++expr;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckSum");
  }
}
template <>
bool& assertPrefixIncr(bool& expr, const char* typeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(expr, typeName, FileName, Line);
  assert(0 && "bool prefix increment is deprecated since C++17");
}

template <typename T>
T assertPostfixIncr(T& expr, const char* typeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckSum<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +expr << ")++ > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, expr++);

  case UBCheckRes::OVERFLOW_MIN:
    assert(0 && "Postfix increment assert detected OVERFLOW_MIN");

  case UBCheckRes::SAFE_OPERATION:
    return expr++;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckSum");
  }
}
template <>
bool assertPostfixIncr(bool& expr, const char* typeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(expr, typeName, FileName, Line);
  assert(0 && "bool postfix increment is deprecated since C++17");
}

template <typename T>
T& assertPrefixDecr(T& expr, const char* typeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckDiff<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    assert(0 && "Prefix decrement assert detected OVERFLOW_MAX");

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: --(" << +expr << ") < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, --expr);

  case UBCheckRes::SAFE_OPERATION:
    return --expr;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiff");
  }
}
template <>
bool& assertPrefixDecr(bool& expr, const char* typeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(expr, typeName, FileName, Line);
  assert(0 && "bool prefix decrement is deprecated since C++17");
}

template <typename T>
T assertPostfixDecr(T& expr, const char* typeName, const char* FileName,
                    int Line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckDiff<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    assert(0 && "Postfix decrement assert detected OVERFLOW_MAX");

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << FileName << " Line: " << Line
              << "\nlog: (" << +expr << ")-- < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(T, expr--);

  case UBCheckRes::SAFE_OPERATION:
    return expr--;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiff");
  }
}
template <>
bool assertPostfixDecr(bool& expr, const char* typeName, const char* FileName,
                       int Line) {
  UNUSED_ASSERT_ARGS(expr, typeName, FileName, Line);
  assert(0 && "bool postfix decrement is deprecated since C++17");
}

} // namespace ub_tester
