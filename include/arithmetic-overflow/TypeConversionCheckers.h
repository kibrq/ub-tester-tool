#pragma once

#include "ArithmeticUBUtility.h"

namespace ub_tester::type_conv {

enum class TyCoCheckRes {
  SAFE_CONVERSION,
  NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION,
  EXPR_OVERFLOWS_TOTYPE_MAX,
  EXPR_OVERFLOWS_TOTYPE_MIN,
  EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED, // until C++20
  EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED, // until C++20
  BOOL_CONVERSION_IS_NOT_CONSIDERED       // because it isn't supposed to cause
                                          // problems for user
};

namespace support {

template <typename CommonType, typename ToType>
TyCoCheckRes checkIfExprFitsInTypeLimits(CommonType CommonTypeExpr) {
  static_assert(!std::is_same<CommonType, bool>::value);
  static_assert(!std::is_same<ToType, bool>::value);
  static_assert(std::numeric_limits<CommonType>::is_integer);
  static_assert(std::numeric_limits<ToType>::is_integer);

  CommonType ToTypeMax =
      static_cast<CommonType>(std::numeric_limits<ToType>::max());
  CommonType ToTypeMin =
      std::numeric_limits<CommonType>::is_signed
          ? static_cast<CommonType>(std::numeric_limits<ToType>::min())
          : 0; // neg value to unsigned ToType conversion case is already
               // considered in checkIntegralConv before
  bool ToTypeIsSigned = std::numeric_limits<ToType>::is_signed;

#if __cplusplus <= 201703L
  // if ToType is signed and expr can't be represented in it =>
  // => implementation defined until C++20
  if (CommonTypeExpr > ToTypeMax && ToTypeIsSigned)
    return TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX_IMPL_DEFINED;
  if (CommonTypeExpr < ToTypeMin && ToTypeIsSigned)
    return TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN_IMPL_DEFINED;
#endif

  if (CommonTypeExpr > ToTypeMax)
    return TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MAX;
  if (CommonTypeExpr < ToTypeMin)
    return TyCoCheckRes::EXPR_OVERFLOWS_TOTYPE_MIN;

  return TyCoCheckRes::SAFE_CONVERSION;
}

} // namespace support

template <typename FromType, typename ToType>
struct Conversions final {
  static TyCoCheckRes checkIntegralConv(FromType Expr) {
    static_assert(std::numeric_limits<FromType>::is_integer);
    static_assert(std::numeric_limits<ToType>::is_integer);

    constexpr bool FromTypeIsSigned = std::numeric_limits<FromType>::is_signed;
    constexpr bool ToTypeIsSigned = std::numeric_limits<ToType>::is_signed;

    if (FromTypeIsSigned && (!ToTypeIsSigned) && Expr < 0)
      return TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION;

    using support::checkIfExprFitsInTypeLimits;
    // compare expr with ToType limits, using the widest signed common type
    if constexpr (FromTypeIsSigned && ToTypeIsSigned) {
      // make_signed is needed to prevent unwanted template instantation
      if (arithm_util::checkIfFirstTypeIsWider<FromType, ToType>())
        return checkIfExprFitsInTypeLimits<FromType, ToType>(
            static_cast<FromType>(Expr));
      return checkIfExprFitsInTypeLimits<ToType, ToType>(
          static_cast<ToType>(Expr));
    }
    // compare expr with ToType limits, using the widest unsigned common type
    using UnsignedFromType = typename std::make_unsigned<FromType>::type;
    using UnsignedToType = typename std::make_unsigned<ToType>::type;
    if (arithm_util::checkIfFirstTypeIsWider<UnsignedFromType,
                                             UnsignedToType>())
      return checkIfExprFitsInTypeLimits<UnsignedFromType, ToType>(
          static_cast<UnsignedFromType>(Expr));
    return checkIfExprFitsInTypeLimits<UnsignedToType, ToType>(
        static_cast<UnsignedToType>(Expr));
  }
};
template <typename ToType>
struct Conversions<bool, ToType> final {
  static TyCoCheckRes checkIntegralConv(bool Expr) noexcept {
    static_cast<void>(Expr);
    return TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED;
  }
};
template <typename FromType>
struct Conversions<FromType, bool> final {
  static TyCoCheckRes checkIntegralConv(FromType Expr) noexcept {
    static_cast<void>(Expr);
    return TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED;
  }
};
template <>
struct Conversions<bool, bool> final {
  static TyCoCheckRes checkIntegralConv(bool Expr) noexcept {
    static_cast<void>(Expr);
    return TyCoCheckRes::BOOL_CONVERSION_IS_NOT_CONSIDERED;
  }
};

} // namespace ub_tester::type_conv
