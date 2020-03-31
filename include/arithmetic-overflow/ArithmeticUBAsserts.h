#pragma once

#include "ArithmeticUBCheckers.h"
#include <cstring>
#include <iostream>

// lhs and rhs can be put in Assert-functions as strings to improve error-log
/* Binary operations usually have equal lhs and rhs types, but there are some
 * exceptions like bitshift operators (can have different integer types).
 * But return type of binary operator is always lhs type. */
#define ASSERT_BINOP(operation, lhs, rhs, lhsType, rhsType)                    \
  Assert##operation<lhsType, rhsType>(                                         \
      (lhs), (rhs), #lhsType, __FILE__, __LINE__)
#define ASSERT_UNOP(operation, expr, type)                                     \
  Assert##operation<type>((expr), #type, __FILE__, __LINE__)

#define OVERFLOW_ASSERT_FAILED(type, returnExprIfWarning)                      \
  if (!std::numeric_limits<type>::is_signed) {                                 \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE, returnExprIfWarning);         \
  } else                                                                       \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

// in future failures and warnings will be collected by special class
#define ASSERT_FAILED(exitCode) /*exit(exitCode)*/ return 0 // for testing
#define PUSH_WARNING(warningCode, returnExprIfWarning)                         \
  std::cerr << "Warning has been generated.\n";                                \
  return returnExprIfWarning;

#define ARE_SAME_TYPES(type1, type2)                                           \
  typedef std::is_same<type1, type2> areSameTypes__;                           \
  assert(areSameTypes__::value)
// will be removed in future, when class for message-args appears
#define UNUSED_ASSERT_ARGS(arg1, arg2, arg3, arg4)                             \
  (void)arg1;                                                                  \
  (void)arg2;                                                                  \
  (void)arg3;                                                                  \
  (void)arg4;

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
LhsType AssertSum(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckSum<LhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs + rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs + rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs + rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckSum");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertDiff(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckDiff<LhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs - rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs - rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs - rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiff");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertMul(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckMul<LhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs * rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs * rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs * rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckMul");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertDiv(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(LhsType);
  ARE_SAME_TYPES(LhsType, RhsType);

  // check for flt-point in future: minLim <= (lhs / 0-approx rhs) <= maxLim
  switch (UBCheckDiv<LhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " / " << +rhs << " > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs / rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " / " << +rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs / rhs);

  case UBCheckRes::DIV_BY_0:
    std::cerr << lhsTypeName << " division by 0 in " << fileName
              << " line: " << line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case UBCheckRes::SAFE_OPERATION:
    return lhs / rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiv");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertMod(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  ARE_SAME_TYPES(LhsType, RhsType);

  switch (UBCheckMod<LhsType>(lhs, rhs)) {
  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << lhsTypeName << " mod (%) is undefined in " << fileName
              << " line: " << line
              << "\nlog: because division is undefined; overflow: " << +lhs
              << " / " << +rhs << " > " << +std::numeric_limits<LhsType>::max()
              << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    std::cerr << lhsTypeName << " mod (%) is undefined in " << fileName
              << " line: " << line
              << "\nlog: because division is undefined; overflow: " << +lhs
              << " / " << +rhs << " < "
              << +std::numeric_limits<LhsType>::lowest() << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case UBCheckRes::DIV_BY_0:
    std::cerr << lhsTypeName << " mod (%) by 0 in " << fileName
              << " line: " << line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);

  case UBCheckRes::SAFE_OPERATION:
    return lhs % rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckMod");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertBitShiftLeft(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;

  switch (UBCheckBitShiftLeft<LhsType, RhsType>(lhs, rhs)) {
  case UBCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << lhsTypeName << " bitshift left (<<) is undefined in "
              << fileName << " line: " << line << "\nlog: negative rhs; "
              << +rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << lhsTypeName << " bitshift left (<<) is undefined in "
              << fileName << " line: " << line
              << "\nlog: rhs >= number of bits in lhs type; " << +rhs
              << " >= " << +(sizeof(LhsType) * CHAR_BIT) << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS:
    std::cerr << lhsTypeName << " bitshift left (<<) is undefined in "
              << fileName << " line: " << line << "\nlog: negative lhs; "
              << +lhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::
      BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS:
    std::cerr << lhsTypeName << " bitshift left (<<) undefined is in "
              << fileName << " line: " << line
              << "\nlog: signed lhs is non-negative, but result is not "
                 "representable in unsigned version of LhsType; ("
              << +lhs << " << " << +rhs << ") > "
              << std::numeric_limits<UnsignedLhsType>::max() << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_LEFT_EXIT_CODE);
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: (" << +lhs << " << " << +rhs << ") > "
              << +std::numeric_limits<LhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(LhsType, lhs << rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs << rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckBitShiftLeft");
  }
}

template <typename LhsType, typename RhsType>
LhsType AssertBitShiftRight(
    LhsType lhs, RhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);

  switch (UBCheckBitShiftRight<LhsType, RhsType>(lhs, rhs)) {
  case UBCheckRes::BITSHIFT_NEGATIVE_RHS:
    std::cerr << lhsTypeName << " bitshift right (>>) is undefined in "
              << fileName << " line: " << line << "\nlog: negative rhs; "
              << +rhs << " < 0\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);
  case UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS:
    std::cerr << lhsTypeName << " bitshift right (>>) is undefined in "
              << fileName << " line: " << line
              << "\nlog: rhs >= number of bits in lhs type; " << +rhs
              << " >= " << +(sizeof(LhsType) * CHAR_BIT) << "\n";
    ASSERT_FAILED(UNDEFINED_BITSHIFT_RIGHT_EXIT_CODE);
  case UBCheckRes::IMPL_DEFINED_OPERATION:
    std::cerr << lhsTypeName
              << " bitshift right (>>) is implementation-defined in "
              << fileName << " line: " << line << "\nlog: negative lhs; "
              << +lhs << " < 0\n";
    PUSH_WARNING(IMPL_DEFINED_WARNING_CODE, lhs >> rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs >> rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckBitShiftRight");
  }
}

template <typename T>
T AssertUnaryNeg(T expr, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckUnaryNeg<T>(expr)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: -(" << +expr << ") > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(T, -expr);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
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
T& AssertPrefixIncr(
    T& expr, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckSum<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
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
bool& AssertPrefixIncr(
    bool& expr, const char* typeName, const char* fileName, int line) {
  UNUSED_ASSERT_ARGS(expr, typeName, fileName, line);
  assert(0 && "bool prefix increment is deprecated since C++17");
}

template <typename T>
T AssertPostfixIncr(
    T& expr, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckSum<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
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
bool AssertPostfixIncr(
    bool& expr, const char* typeName, const char* fileName, int line) {
  UNUSED_ASSERT_ARGS(expr, typeName, fileName, line);
  assert(0 && "bool postfix increment is deprecated since C++17");
}

template <typename T>
T& AssertPrefixDecr(
    T& expr, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckDiff<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    assert(0 && "Prefix decrement assert detected OVERFLOW_MAX");

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
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
bool& AssertPrefixDecr(
    bool& expr, const char* typeName, const char* fileName, int line) {
  UNUSED_ASSERT_ARGS(expr, typeName, fileName, line);
  assert(0 && "bool prefix decrement is deprecated since C++17");
}

template <typename T>
T AssertPostfixDecr(
    T& expr, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED(T);

  switch (UBCheckDiff<T>(expr, 1)) {
  case UBCheckRes::OVERFLOW_MAX:
    assert(0 && "Postfix decrement assert detected OVERFLOW_MAX");

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
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
bool AssertPostfixDecr(
    bool& expr, const char* typeName, const char* fileName, int line) {
  UNUSED_ASSERT_ARGS(expr, typeName, fileName, line);
  assert(0 && "bool postfix decrement is deprecated since C++17");
}

} // namespace ub_tester
