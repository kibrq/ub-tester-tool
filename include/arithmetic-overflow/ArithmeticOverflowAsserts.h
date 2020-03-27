#pragma once

#include <iostream>
#include <limits>

#define ASSERT_OVERFLOW(operation, lhs, rhs, type)                             \
  Assert##operation<type>((lhs), (rhs), #type, __FILE__, __LINE__)

#define ASSERT_FAILED(exitCode) exit(exitCode) // return 0 for testing

namespace ub_tester {

constexpr int OVERFLOW_EXIT_CODE = -1; // should be moved to others exit-codes

template <typename T>
T AssertSum(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  if (rhs > 0 && lhs > std::numeric_limits<T>::max() - rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " + " << rhs << " > "
              << std::numeric_limits<T>::max() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  if (rhs < 0 && lhs < std::numeric_limits<T>::lowest() - rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " + " << rhs << " < "
              << std::numeric_limits<T>::lowest() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  return lhs + rhs;
}

template <typename T>
T AssertDiff(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  if (rhs > 0 && lhs < std::numeric_limits<T>::lowest() + rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " - " << rhs << " < "
              << std::numeric_limits<T>::lowest() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  if (rhs < 0 && lhs > std::numeric_limits<T>::max() + rhs) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " - " << rhs << " > "
              << std::numeric_limits<T>::max() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  return lhs - rhs;
}

template <typename T>
bool CheckIntegerDivisibility(T arg1, T arg2) {
  return arg1 % arg2 == 0;
}
// this method is used only for integer types
// for floating types: it returns false and doesn't raise -Wunused-parameter
template <>
bool CheckIntegerDivisibility<float>(float arg1, float arg2) {
  return false && (arg1 == arg2);
}
template <>
bool CheckIntegerDivisibility<double>(double arg1, double arg2) {
  return false && (arg1 == arg2);
}
template <>
bool CheckIntegerDivisibility<long double>(long double arg1, long double arg2) {
  return false && (arg1 == arg2);
}

template <typename T>
T AssertMul(
    T lhs, T rhs, const char* typeName, const char* fileName, int line) {
  if (lhs == 0 || rhs == 0)
    return lhs * rhs;
  T maxLim = std::numeric_limits<T>::max();
  T minLim = std::numeric_limits<T>::min();
  bool isIntegerType = std::numeric_limits<T>::is_integer;
  if ((lhs > 0 && rhs > 0) &&
      (lhs > maxLim / rhs ||
       (lhs == maxLim / rhs &&
        ((!isIntegerType) || CheckIntegerDivisibility(maxLim, rhs))))) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " * " << rhs << " > "
              << std::numeric_limits<T>::max() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  if (!std::numeric_limits<T>::is_signed)
    return lhs * rhs;
  if ((lhs < 0 && rhs > 0) &&
      (lhs < minLim / rhs ||
       (lhs == minLim / rhs &&
        ((!isIntegerType) || CheckIntegerDivisibility(minLim, rhs))))) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " * " << rhs << " < "
              << std::numeric_limits<T>::min() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  if ((lhs > 0 && rhs < 0) &&
      (lhs > minLim / rhs ||
       (lhs == minLim / rhs &&
        ((!isIntegerType) || CheckIntegerDivisibility(minLim, rhs))))) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " * " << rhs << " < "
              << std::numeric_limits<T>::min() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  if ((lhs < 0 && rhs < 0) &&
      (lhs < maxLim / rhs ||
       (lhs == maxLim / rhs &&
        ((!isIntegerType) || CheckIntegerDivisibility(maxLim, rhs))))) {
    std::cerr << typeName << " overflow in " << fileName << " line: " << line
              << "\nlog: " << lhs << " * " << rhs << " > "
              << std::numeric_limits<T>::max() << "\n";
    ASSERT_FAILED(OVERFLOW_EXIT_CODE);
  }
  return lhs * rhs;
}

} // namespace ub_tester
