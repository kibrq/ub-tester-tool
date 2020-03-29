#pragma once

#include <cassert>
#include <iostream>
#include <limits>

// lhs and rhs can be put in Assert-functions as strings to improve error-log
#define ASSERT_OVERFLOW(operation, lhs, rhs, type)                             \
  Assert##operation<type>((lhs), (rhs), #type, __FILE__, __LINE__)

#define OVERFLOW_ASSERT_FAILED                                                 \
  if (!std::numeric_limits<T>::is_signed)                                      \
    PUSH_WARNING(UNSIGNED_OVERFLOW_WARNING_CODE);                              \
  else                                                                         \
    ASSERT_FAILED(OVERFLOW_EXIT_CODE)

#define ASSERT_FAILED(exitCode) /*exit(exitCode)*/ return 0 // for testing
#define PUSH_WARNING(warningCode) std::cerr << "Warning has been generated.\n"

#define FLT_POINT_NOT_SUPPORTED assert(std::numeric_limits<T>::is_integer)

namespace ub_tester {

// should be moved to others exit-codes
constexpr int OVERFLOW_EXIT_CODE = -1;
constexpr int DIVISION_BY_ZERO_EXIT_CODE = -2;
constexpr int INVALID_BIT_SHIFT_RHS_EXIT_CODE = -3;

constexpr int UNSIGNED_OVERFLOW_WARNING_CODE = -4;
constexpr int UNDERFLOW_WARNING_CODE = -5; // isn't supported for now

template <typename T>
T AssertSum(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED;
  if (rhs > 0 && lhs > std::numeric_limits<T>::max() - rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  if (rhs < 0 && lhs < std::numeric_limits<T>::lowest() - rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " + " << +rhs << " < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  return lhs + rhs;
}

template <typename T>
T AssertDiff(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED;
  if (rhs > 0 && lhs < std::numeric_limits<T>::lowest() + rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " < "
              << +std::numeric_limits<T>::lowest() << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  if (rhs < 0 && lhs > std::numeric_limits<T>::max() + rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " - " << +rhs << " > "
              << +std::numeric_limits<T>::max() << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  return lhs - rhs;
}

template <typename T>
T AssertMul(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED;
  if (lhs == 0 || rhs == 0)
    return lhs * rhs;
  T maxLim = std::numeric_limits<T>::max();
  T minLim = std::numeric_limits<T>::lowest();
  bool isIntegerType = std::numeric_limits<T>::is_integer;
  // special case of rhs = -1 (then division lim / rhs can overflow)
  if (std::numeric_limits<T>::is_signed && rhs == static_cast<T>(-1)) {
    T tmp = minLim + lhs;
    if (lhs > 0 && tmp > 0) {
      std::cerr << typeName << " overflow in " << fileName << " line: " << line
                << "\nlog: " << +lhs << " * " << +rhs << " < " << +minLim
                << "\n";
      OVERFLOW_ASSERT_FAILED;
    }
    if (lhs < 0 && maxLim + lhs < 0) {
      std::cerr << typeName << " overflow in " << fileName << " line: " << line
                << "\nlog: " << +lhs << " * " << +rhs << " > " << +maxLim
                << "\n";
      OVERFLOW_ASSERT_FAILED;
    }
    return lhs * rhs;
  }
  if ((lhs > 0 && rhs > 0) && lhs > maxLim / rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " > " << +maxLim << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  if (!std::numeric_limits<T>::is_signed)
    return lhs * rhs;
  if ((lhs < 0 && rhs > 0) && lhs < minLim / rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " < " << +minLim << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  if ((lhs > 0 && rhs < 0) && lhs > minLim / rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " < " << +minLim << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  if ((lhs < 0 && rhs < 0) && lhs < maxLim / rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " * " << +rhs << " > " << +maxLim << "\n";
    OVERFLOW_ASSERT_FAILED;
  }
  // this check doesn't work for now because flt-point types are not supported
  if ((!isIntegerType) && lhs * rhs == 0 && lhs != 0 && rhs != 0) {
    std::cerr << typeName << "underflow in " << fileName << " line: " << line
              << "\nlog: " << +lhs << " *" << +rhs << " = 0"
              << " caused by " << typeName << " precision\n";
    PUSH_WARNING(UNDERFLOW_WARNING_CODE);
  }
  return lhs * rhs;
}

template <typename T>
T AssertDiv(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  FLT_POINT_NOT_SUPPORTED;
  if (rhs == 0) {
    std::cerr << typeName << " division by 0 in " << fileName
              << " line: " << line << "\n";
    ASSERT_FAILED(DIVISION_BY_ZERO_EXIT_CODE);
  }
  if (std::numeric_limits<T>::is_integer) { // support -1 in future
    return lhs / rhs;
  }
  // in future it's need to be check minLim <= lhs / small rhs <= maxLim
  return lhs / rhs;
}

template <typename T>
T AssertBitShiftLeft(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  assert(std::numeric_limits<T>::is_integer);
  if (rhs < 0) {
    std::cerr << "invalid rhs argument for << operator in " << fileName
              << " line: " << line << "\nlog: " << +rhs << " < 0\n";
    ASSERT_FAILED(INVALID_BIT_SHIFT_RHS_EXIT_CODE);
  }
  if (rhs >= static_cast<T>(sizeof(T) * 8)) {
    std::cerr << "invalid rhs argument for << operator in " << fileName
              << " line: " << line << "\nlog: " << +rhs << " >= "
              << "number of bits in type " << typeName << ": " << +sizeof(T) * 8
              << "\n";
    ASSERT_FAILED(INVALID_BIT_SHIFT_RHS_EXIT_CODE);
  }
  return lhs << rhs;
}

} // namespace ub_tester
