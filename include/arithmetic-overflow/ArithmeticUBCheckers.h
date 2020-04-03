#pragma once

#include <cassert>
#include <climits>
#include <limits>
#include <type_traits>

#define FLT_POINT_NOT_SUPPORTED(Type)                                          \
  assert(std::numeric_limits<Type>::is_integer)

namespace ub_tester {

enum class UBCheckRes {             // can be used as return from:
  OVERFLOW_MAX,                     // all operators
  OVERFLOW_MIN,                     // all operators
  SAFE_OPERATION,                   // all operators
  IMPL_DEFINED_OPERATION,           // all operators
  DIV_BY_0,                         // only / and %
  MOD_UNDEFINED_DIV_OVERFLOWS_MAX,  // only %
  MOD_UNDEFINED_DIV_OVERFLOWS_MIN,  // only %
  BITSHIFT_NEGATIVE_RHS,            // only << and >>
  BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS, // only << and >>
  BITSHIFT_LEFT_NEGATIVE_LHS,       // only <<
  BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS // only <<
};

/* ArithmeticUBCheckers only detect problem, then ArithmeticUBAsserts decide is
 * it warning or error: for example, unsigned overflow is detected as OVERFLOW,
 * then ArithmeticUBAsserts push warning, not error. */

template <typename T>
UBCheckRes UBCheckSum(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs > 0 && Lhs > std::numeric_limits<T>::max() - Rhs)
    return UBCheckRes::OVERFLOW_MAX;
  if (Rhs < 0 && Lhs < std::numeric_limits<T>::lowest() - Rhs)
    return UBCheckRes::OVERFLOW_MIN;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckDiff(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs > 0 && Lhs < std::numeric_limits<T>::lowest() + Rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if (Rhs < 0 && Lhs > std::numeric_limits<T>::max() + Rhs)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckMul(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Lhs == 0 || Rhs == 0)
    return UBCheckRes::SAFE_OPERATION;
  T MaxLim = std::numeric_limits<T>::max();
  T MinLim = std::numeric_limits<T>::lowest();

  // special case of Rhs = -1 (then division lim / Rhs can overflow)
  if (std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)) {
    if (Lhs > 0 && MinLim + Lhs > 0)
      return UBCheckRes::OVERFLOW_MIN;
    if (Lhs < 0 && MaxLim + Lhs < 0)
      return UBCheckRes::OVERFLOW_MAX;
    return UBCheckRes::SAFE_OPERATION;
  }
  if ((Lhs > 0 && Rhs > 0) && Lhs > MaxLim / Rhs)
    return UBCheckRes::OVERFLOW_MAX;
  if (!std::numeric_limits<T>::is_signed)
    return UBCheckRes::SAFE_OPERATION; // quick return for unsigned T
  if ((Lhs < 0 && Rhs > 0) && Lhs < MinLim / Rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if ((Lhs > 0 && Rhs < 0) && Lhs > MinLim / Rhs)
    return UBCheckRes::OVERFLOW_MIN;
  if ((Lhs < 0 && Rhs < 0) && Lhs < MaxLim / Rhs)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckDiv(T Lhs, T Rhs) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Rhs == 0)
    return UBCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)))
    return UBCheckRes::SAFE_OPERATION;

  // only case when integer div can overflow: Rhs == -1
  if (Lhs > 0 && std::numeric_limits<T>::lowest() + Lhs > 0)
    return UBCheckRes::OVERFLOW_MIN;
  if (Lhs < 0 && std::numeric_limits<T>::max() + Lhs < 0)
    return UBCheckRes::OVERFLOW_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckMod(T Lhs, T Rhs) {
  assert(std::numeric_limits<T>::is_integer);
  if (Rhs == 0)
    return UBCheckRes::DIV_BY_0;
  if (!(std::numeric_limits<T>::is_signed && Rhs == static_cast<T>(-1)))
    return UBCheckRes::SAFE_OPERATION;

  // only case when integer mod is undefined: Rhs == -1 (because div overflows)
  if (Lhs > 0 && std::numeric_limits<T>::lowest() + Lhs > 0)
    return UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MIN;
  if (Lhs < 0 && std::numeric_limits<T>::max() + Lhs < 0)
    return UBCheckRes::MOD_UNDEFINED_DIV_OVERFLOWS_MAX;

  return UBCheckRes::SAFE_OPERATION;
}

template <typename LhsType, typename RhsType>
UBCheckRes UBCheckBitShiftLeft(LhsType Lhs, RhsType Rhs) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);

  if (Rhs < 0)
    return UBCheckRes::BITSHIFT_NEGATIVE_RHS;
  if (Rhs >= static_cast<RhsType>(sizeof(LhsType) * 8))
    return UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS;

// since C++20 all other cases are SAFE_OPERATION
// but now C++17 is considered
#if __cplusplus > 201703L
  return UBCheckRes::SAFE_OPERATION;
#endif

  if (Lhs < 0)
    return UBCheckRes::BITSHIFT_LEFT_NEGATIVE_LHS;

  // check if Lhs * (1 << Rhs) is representable in unsigned Lhs type
  typedef typename std::make_unsigned<LhsType>::type UnsignedLhsType;
  if (UBCheckMul<UnsignedLhsType>(static_cast<UnsignedLhsType>(Lhs),
                                  static_cast<UnsignedLhsType>(1) << Rhs) !=
      UBCheckRes::SAFE_OPERATION) {
    if (std::numeric_limits<LhsType>::is_signed) // for signed res is undefined
      return UBCheckRes::
          BITSHIFT_LEFT_RES_OVERFLOWS_UNSIGNED_MAX_FOR_NONNEG_SIGNED_LHS;
    return UBCheckRes::OVERFLOW_MAX; // for unsigned occurs overflow
  }

  return UBCheckRes::SAFE_OPERATION;
}

template <typename LhsType, typename RhsType>
UBCheckRes UBCheckBitShiftRight(LhsType Lhs, RhsType Rhs) {
  assert(std::numeric_limits<LhsType>::is_integer);
  assert(std::numeric_limits<RhsType>::is_integer);

  if (Rhs < 0)
    return UBCheckRes::BITSHIFT_NEGATIVE_RHS;
  if (Rhs >= static_cast<RhsType>(sizeof(LhsType) * 8))
    return UBCheckRes::BITSHIFT_RHS_GEQ_LHSTYPE_IN_BITS;

// since C++20 all other cases are SAFE_OPERATION
// but now C++17 is considered
#if __cplusplus > 201703L
  return UBCheckRes::SAFE_OPERATION;
#endif

  if (Lhs < 0)
    return UBCheckRes::IMPL_DEFINED_OPERATION;
  return UBCheckRes::SAFE_OPERATION;
}

template <typename T>
UBCheckRes UBCheckUnaryNeg(T Expr) {
  FLT_POINT_NOT_SUPPORTED(T);
  if (Expr > 0 && std::numeric_limits<T>::lowest() + Expr > 0)
    return UBCheckRes::OVERFLOW_MIN;
  if (Expr < 0 && std::numeric_limits<T>::max() + Expr < 0)
    return UBCheckRes::OVERFLOW_MAX;
  return UBCheckRes::SAFE_OPERATION;
}

} // namespace ub_tester
