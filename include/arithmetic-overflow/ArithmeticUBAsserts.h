#pragma once

#include "ArithmeticUBCheckers.h"
#include <cstring>
#include <iostream>
#include <type_traits>

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
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE);                              \
    return returnExprIfWarning;                                                \
  } else                                                                       \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

// in future failures and warnings will be collected by special class
#define ASSERT_FAILED(exitCode) /*exit(exitCode)*/ return 0 // for testing
#define PUSH_WARNING(warningCode) std::cerr << "Warning has been generated.\n";

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
constexpr int INVALID_BIT_SHIFT_RHS_EXIT_CODE = -4;

constexpr int UNSIGNED_OVERFLOW_WARNING_CODE = -5;

template <typename lhsType, typename rhsType>
lhsType AssertSum(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(lhsType);
  ARE_SAME_TYPES(lhsType, rhsType);

  switch (UBCheckSum<lhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " > "
              << +std::numeric_limits<lhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs + rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " < "
              << +std::numeric_limits<lhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs + rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs + rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckSum");
  }
}

template <typename lhsType, typename rhsType>
lhsType AssertDiff(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(lhsType);
  ARE_SAME_TYPES(lhsType, rhsType);

  switch (UBCheckDiff<lhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " > "
              << +std::numeric_limits<lhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs - rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " < "
              << +std::numeric_limits<lhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs - rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs - rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckDiff");
  }
}

template <typename lhsType, typename rhsType>
lhsType AssertMul(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(lhsType);
  ARE_SAME_TYPES(lhsType, rhsType);

  switch (UBCheckMul<lhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " > "
              << +std::numeric_limits<lhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs * rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " < "
              << +std::numeric_limits<lhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs * rhs);

  case UBCheckRes::SAFE_OPERATION:
    return lhs * rhs;
  default:
    assert(0 && "Unexpected UBCheckRes from UBCheckMul");
  }
}

template <typename lhsType, typename rhsType>
lhsType AssertDiv(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  FLT_POINT_NOT_SUPPORTED(lhsType);
  ARE_SAME_TYPES(lhsType, rhsType);

  // check for flt-point in future: minLim <= (lhs / 0-approx rhs) <= maxLim
  switch (UBCheckDiv<lhsType>(lhs, rhs)) {
  case UBCheckRes::OVERFLOW_MAX:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " / " << +rhs << " > "
              << +std::numeric_limits<lhsType>::max() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs / rhs);

  case UBCheckRes::OVERFLOW_MIN:
    std::cerr << lhsTypeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " / " << +rhs << " < "
              << +std::numeric_limits<lhsType>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED(lhsType, lhs / rhs);

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

template <typename lhsType, typename rhsType>
lhsType AssertMod(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  assert(std::numeric_limits<lhsType>::is_integer);
  ARE_SAME_TYPES(lhsType, rhsType);

  switch (UBCheckMod<lhsType>(lhs, rhs)) {
  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX:
    std::cerr << lhsTypeName << " mod (%) is undefined in " << fileName
              << " line: " << line
              << "\nlog: because division is undefined; overflow: " << +lhs
              << " / " << +rhs << " > " << +std::numeric_limits<lhsType>::max()
              << "\n";
    ASSERT_FAILED(UNDEFINED_MOD_EXIT_CODE);

  case UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN:
    std::cerr << lhsTypeName << " mod (%) is undefined in " << fileName
              << " line: " << line
              << "\nlog: because division is undefined; overflow: " << +lhs
              << " / " << +rhs << " < "
              << +std::numeric_limits<lhsType>::lowest() << "\n";
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

template <typename lhsType, typename rhsType>
lhsType AssertBitShiftLeft(
    lhsType lhs, rhsType rhs, const char* lhsTypeName, const char* fileName,
    int line) {
  assert(std::numeric_limits<lhsType>::is_integer);
  return lhs << rhs;
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
