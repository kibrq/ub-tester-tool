#pragma once

#include "ArithmeticUBUtility.h"

namespace ub_tester {
namespace type_conv {

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

namespace type_conv_support {

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

} // namespace type_conv_support

template <typename FromType, typename ToType>
struct Conversions final {
  static TyCoCheckRes checkIntegralConv(FromType Expr) {
    static_assert(std::numeric_limits<FromType>::is_integer);
    static_assert(std::numeric_limits<ToType>::is_integer);

    bool FromTypeIsSigned = std::numeric_limits<FromType>::is_signed;
    bool ToTypeIsSigned = std::numeric_limits<ToType>::is_signed;

    if (FromTypeIsSigned && (!ToTypeIsSigned) && Expr < 0)
      return TyCoCheckRes::NEG_VALUE_TO_UNSIGNED_TYPE_CONVERSION;

    using type_conv_support::checkIfExprFitsInTypeLimits;
    // compare expr with ToType limits, using the widest signed common type
    if (FromTypeIsSigned && ToTypeIsSigned) {
      // make_signed is needed to prevent unwanted template instantation
      typedef typename std::make_signed<FromType>::type SFromType;
      typedef typename std::make_signed<ToType>::type SToType;
      if (arithm_ut::checkIfFirstTypeIsWider<SFromType, SToType>())
        return checkIfExprFitsInTypeLimits<SFromType, ToType>(
            static_cast<SFromType>(Expr));
      return checkIfExprFitsInTypeLimits<SToType, ToType>(
          static_cast<SToType>(Expr));
    }
    // compare expr with ToType limits, using the widest unsigned common type
    typedef typename std::make_unsigned<FromType>::type UnsignedFromType;
    typedef typename std::make_unsigned<ToType>::type UnsignedToType;
    if (arithm_ut::checkIfFirstTypeIsWider<UnsignedFromType, UnsignedToType>())
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

} // namespace type_conv
} // namespace ub_tester
