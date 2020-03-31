#pragma once

#include <cassert>
#include <climits>
#include <limits>
#include <type_traits>

#define FLT_POINT_NOT_SUPPORTED(type)                                          \
  assert(std::numeric_limits<type>::is_integer)

namespace ub_tester {

enum class UBCheckRes {
  OVERFLOW_MAX,                     // all operators
  OVERFLOW_MIN,                     // all operators
  SAFE_OPERATION,                   // all operators
  DIV_BY_0,                         // only / and %
  MOD_UNDEFINED_DIV_OVERFLOWS_MAX,  // only %
  MOD_UNDEFINED_DIV_OVERFLOWS_MIN,  // only %
  BITSHIFT_NEGATIVE_RHS,            // only for << and >>
  BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS, // only for << and >>
  BITSHIFT_LEFT_NEGATIVE_LHS,       // only for <<
  BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS // only for <<
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
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);

  if (rhs < 0)
    return UBCheckRes::BITSHIFT_NEGATIVE_RHS;
  if (rhs >= static_cast<RhsType>(sizeof(LhsType) * 8))
    return UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS;

// since C++20 all other cases are SAFE_OPERATION
// but now C++17 is considered
#if __cplusplus > 201703L
  return UBCheckRes::SAFE_OPERATION;
#endif

  if (lhs < 0)
    return UBCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS;

  // check if lhs * (1 << rhs) is representable in unsigned lhs type
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;
  if (UBCheckMul<UnsignedLhsType>(
          static_cast<UnsignedLhsType>(lhs), static_cast<UnsignedLhsType>(1)
                                                 << rhs) !=
      UBCheckRes::SAFE_OPERATION) {
    if (std::numeric_limits<LhsType>::is_signed) // for signed res is undefined
      return UBCheckRes::
          BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS;
    return UBCheckRes::OVERFLOW_MAX; // for unsigned occurs overflow
  }

  return UBCheckRes::SAFE_OPERATION;
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
