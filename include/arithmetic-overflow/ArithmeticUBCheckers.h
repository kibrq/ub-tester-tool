#pragma once

#include <cassert>
#include <limits>

#define FLT_POINT_NOT_SUPPORTED(type)                                          \
  assert(std::numeric_limits<type>::is_integer)

namespace ub_tester {

enum class UBCheckRes {
  OVERFLOW_MAX,                    // all operators
  OVERFLOW_MIN,                    // all operators
  SAFE_OPERATION,                  // all operators
  DIV_BY_0,                        // only / and %
  MOD_UNDEFINED_DIV_OVERFLOWS_MAX, // only %
  MOD_UNDEFINED_DIV_OVERFLOWS_MIN  // only %
};

/* ArithmeticUBCheckers only detect problem, then ArithmeticUBAsserts decide is
 * it warning or error: for example, unsigned overflow is detected as OVERFLOW,
 * then ArithmeticUBAsserts push warning, not error. */

template <typename T>
UBCheckRes UBCheckSum(T lhs, T rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (rhs > 0 && lhs > std::numeric_limits<T>::max() - rhs)
    return UBCheckRes::OVERFLOW_MAX;
  if (rhs < 0 && lhs < std::numeric_limits<T>::lowest() - rhs)
    return UBCheckRes::OVERFLOW_MIN;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckDiff(T lhs, T rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (rhs > 0 && lhs < std::numeric_limits<T>::lowest() + rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if (rhs < 0 && lhs > std::numeric_limits<T>::max() + rhs)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckMul(T lhs, T rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (lhs == 0 || rhs == 0)
    return UBCheckRes::SAFE_OPERATION;
  T maxLim = std::numeric_limits<T>::max();
  T minLim = std::numeric_limits<T>::lowest();

  // special case of rhs = -1 (then division lim / rhs can overflow)
  if (std::numeric_limits<T>::is_signed && rhs == static_cast<T>(-1)) {
    if (lhs > 0 && minLim + lhs > 0)
      return UBCheckRes::OVERFLOW_MIN;
    if (lhs < 0 && maxLim + lhs < 0)
      return UBCheckRes::OVERFLOW_MAX;
    return UBCheckRes::SAFE_OPERATION;
  }
  if ((lhs > 0 && rhs > 0) && lhs > maxLim / rhs)
    return UBCheckRes::OVERFLOW_MAX;
  if (!std::numeric_limits<T>::is_signed)
    return UBCheckRes::SAFE_OPERATION; // quick return for unsigned T
  if ((lhs < 0 && rhs > 0) && lhs < minLim / rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if ((lhs > 0 && rhs < 0) && lhs > minLim / rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if ((lhs < 0 && rhs < 0) && lhs < maxLim / rhs)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckDiv(T lhs, T rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (rhs == 0)
    return UBCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && rhs == static_cast<T>(-1)))
    return UBCheckRes::SAFE_OPERATION;

  // only case when integer div can overflow: rhs == -1
  if (lhs > 0 && std::numeric_limits<T>::lowest() + lhs > 0)
    return UBCheckRes::OVERFLOW_MIN;
  if (lhs < 0 && std::numeric_limits<T>::max() + lhs < 0)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckMod(T lhs, T rhs) {
  assert(std::numeric_limits<T>::is_integer);
  if (rhs == 0)
    return UBCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && rhs == static_cast<T>(-1)))
    return UBCheckRes::SAFE_OPERATION;

  // only case when integer mod is undefined: rhs == -1 (because div overflows)
  if (lhs > 0 && std::numeric_limits<T>::lowest() + lhs > 0)
    return UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN;
  if (lhs < 0 && std::numeric_limits<T>::max() + lhs < 0)
    return UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename LhsType, typename RhsType>
UBCheckRes UBCheckBitShiftLeft(LhsType lhs, RhsType rhs) {
  assert(
      std::numeric_limits<LhsType>::is_integer &&
      std::numeric_limits<RhsType>::is_integer);
  return UBCheckRes::SAFE_OPERATION; // todo
}

template <typename T>
UBCheckRes UBCheckUnaryNeg(T expr) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (expr > 0 && std::numeric_limits<T>::lowest() + expr > 0)
    return UBCheckRes::OVERFLOW_MIN;
  if (expr < 0 && std::numeric_limits<T>::max() + expr < 0)
    return UBCheckRes::OVERFLOW_MAX;
  return UBCheckRes::SAFE_OPERATION;
}

} // namespace ub_tester
