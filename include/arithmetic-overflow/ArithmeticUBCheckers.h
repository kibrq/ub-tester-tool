#pragma once

#include "ArithmeticUBUtility.h"
#include <cassert>
#include <type_traits>

#define FLT_POINT_NOT_SUPPORTED(Type)                                          \
  static_assert(std::numeric_limits<Type>::is_integer)

namespace ub_tester {
namespace arithm_check {

enum class ArithmCheckRes {            // can be used as return from:
  OVERFLOW_MAX,                        // all operators
  OVERFLOW_MIN,                        // all operators
  SAFE_OPERATION,                      // all operators
  IMPL_DEFINED_OPERATION,              // only >>
  DIV_BY_0,                            // only / and %
  MOD_UNDEFINED_DIV_OVERFLOWS_MAX,     // only %
  MOD_UNDEFINED_DIV_OVERFLOWS_MIN,     // only %
  BITSHIFT_NEGATIVE_RHS,               // only << and >>
  BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS,    // only << and >>
  OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20, // only <<
  OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20, // only <<
  BITSHIFT_LEFT_NEGATIVE_LHS,          // only <<
  BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS // only <<
};

/* ArithmeticUBCheckers only detect problem, then ArithmeticUBAsserts decide
 * is it warning or error: for example, unsigned overflow is detected as
 * OVERFLOW, then ArithmeticUBAsserts push warning, not error. */

template <typename T>
ArithmCheckRes checkSum(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs > 0 && Lhs > std::numeric_limits<T>::max() - Rhs)
    return ArithmCheckRes::OVERFLOW_MAX;
  if (Rhs < 0 && Lhs < std::numeric_limits<T>::lowest() - Rhs)
    return ArithmCheckRes::OVERFLOW_MIN;

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename T>
ArithmCheckRes checkDiff(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs > 0 && Lhs < std::numeric_limits<T>::lowest() + Rhs)
    return ArithmCheckRes::OVERFLOW_MIN;
  if (Rhs < 0 && Lhs > std::numeric_limits<T>::max() + Rhs)
    return ArithmCheckRes::OVERFLOW_MAX;

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename T>
ArithmCheckRes checkMul(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Lhs == 0 || Rhs == 0)
    return ArithmCheckRes::SAFE_OPERATION;
  T MaxLim = std::numeric_limits<T>::max();
  T MinLim = std::numeric_limits<T>::lowest();

  // special case of Rhs = -1 (then division lim / Rhs can overflow)
  if (std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)) {
    if (Lhs > 0 && MinLim + Lhs > 0)
      return ArithmCheckRes::OVERFLOW_MIN;
    if (Lhs < 0 && MaxLim + Lhs < 0)
      return ArithmCheckRes::OVERFLOW_MAX;
    return ArithmCheckRes::SAFE_OPERATION;
  }
  if ((Lhs > 0 && Rhs > 0) && Lhs > MaxLim / Rhs)
    return ArithmCheckRes::OVERFLOW_MAX;
  if (!std::numeric_limits<T>::is_signed)
    return ArithmCheckRes::SAFE_OPERATION; // quick return for unsigned T
  if ((Lhs < 0 && Rhs > 0) && Lhs < MinLim / Rhs)
    return ArithmCheckRes::OVERFLOW_MIN;
  if ((Lhs > 0 && Rhs < 0) && Lhs > MinLim / Rhs)
    return ArithmCheckRes::OVERFLOW_MIN;
  if ((Lhs < 0 && Rhs < 0) && Lhs < MaxLim / Rhs)
    return ArithmCheckRes::OVERFLOW_MAX;

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename T>
ArithmCheckRes checkDiv(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs == 0)
    return ArithmCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)))
    return ArithmCheckRes::SAFE_OPERATION;

  // only case when integer div can overflow: Rhs == -1
  if (Lhs > 0 && std::numeric_limits<T>::lowest() + Lhs > 0)
    return ArithmCheckRes::OVERFLOW_MIN;
  if (Lhs < 0 && std::numeric_limits<T>::max() + Lhs < 0)
    return ArithmCheckRes::OVERFLOW_MAX;

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename T>
ArithmCheckRes checkMod(T Lhs, T Rhs) {
  static_assert(std::numeric_limits<T>::is_integer);
  if (Rhs == 0)
    return ArithmCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)))
    return ArithmCheckRes::SAFE_OPERATION;

  // only case when integer mod is undefined: Rhs == -1 (because div overflows)
  if (Lhs > 0 && std::numeric_limits<T>::lowest() + Lhs > 0)
    return ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN;
  if (Lhs < 0 && std::numeric_limits<T>::max() + Lhs < 0)
    return ArithmCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX;

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename LhsType, typename RhsType>
ArithmCheckRes checkBitShiftLeft(LhsType Lhs, RhsType Rhs) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);

  if (Rhs < 0)
    return ArithmCheckRes::BITSHIFT_NEGATIVE_RHS;
  if (Rhs >= static_cast<RhsType>(arithm_ut::getTypeSizeInBits<LhsType>()))
    return ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS;

// since C++20 behaviour in other cases is well-defined
// but overflow warning can be generated
#if __cplusplus > 201703L
  switch (checkMul<LhsType>(Lhs, static_cast<LhsType>(1) << Rhs)) {
  case ArithmCheckRes::OVERFLOW_MAX:
    return ArithmCheckRes::OVERFLOW_MAX_IN_BITSHIFT_LEFT_CXX20;
  case ArithmCheckRes::OVERFLOW_MIN:
    return ArithmCheckRes::OVERFLOW_MIN_IN_BITSHIFT_LEFT_CXX20;
  case ArithmCheckRes::SAFE_OPERATION:
    return ArithmCheckRes::SAFE_OPERATION;
  default:
    assert(0 && "Unexpected ArithmCheckRes from checkMul");
  }
#endif

  if (Lhs < 0)
    return ArithmCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS;

  // check if Lhs * (1 << Rhs) is representable in unsigned Lhs type
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;
  if (checkMul<UnsignedLhsType>(static_cast<UnsignedLhsType>(Lhs),
                                static_cast<UnsignedLhsType>(1) << Rhs) !=
      ArithmCheckRes::SAFE_OPERATION) {
    if (std::numeric_limits<LhsType>::is_signed) // for signed res is undefined
      return ArithmCheckRes::
          BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_WITH_NONNEG_SIGNED_LHS;
    return ArithmCheckRes::OVERFLOW_MAX; // for unsigned occurs overflow
  }

  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename LhsType, typename RhsType>
ArithmCheckRes checkBitShiftRight(LhsType Lhs, RhsType Rhs) {
  static_assert(std::numeric_limits<LhsType>::is_integer);
  static_assert(std::numeric_limits<RhsType>::is_integer);

  if (Rhs < 0)
    return ArithmCheckRes::BITSHIFT_NEGATIVE_RHS;
  if (Rhs >= static_cast<RhsType>(arithm_ut::getTypeSizeInBits<LhsType>()))
    return ArithmCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS;

// since C++20 all other cases are SAFE_OPERATION
// but now C++17 is considered
#if __cplusplus > 201703L
  return ArithmCheckRes::SAFE_OPERATION;
#endif

  if (Lhs < 0)
    return ArithmCheckRes::IMPL_DEFINED_OPERATION;
  return ArithmCheckRes::SAFE_OPERATION;
}

template <typename T>
ArithmCheckRes checkUnaryNeg(T Expr) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Expr > 0 && std::numeric_limits<T>::lowest() + Expr > 0)
    return ArithmCheckRes::OVERFLOW_MIN;
  if (Expr < 0 && std::numeric_limits<T>::max() + Expr < 0)
    return ArithmCheckRes::OVERFLOW_MAX;
  return ArithmCheckRes::SAFE_OPERATION;
}

} // namespace arithm_check
} // namespace ub_tester
