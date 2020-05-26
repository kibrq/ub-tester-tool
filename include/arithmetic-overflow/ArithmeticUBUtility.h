#pragma once

#include <climits>
#include <cstddef>
#include <limits>

#define ARE_BOTH_SIGNED_OR_UNSIGNED(Type1, Type2)                              \
  static_assert(std::numeric_limits<Type1>::is_signed ==                       \
                std::numeric_limits<Type2>::is_signed)

namespace ub_tester {
namespace arithm_ut {

template <typename T>
constexpr size_t getTypeSizeInBits() {
  return sizeof(T) * CHAR_BIT;
}

template <typename FirstType, typename SecondType>
constexpr bool checkIfFirstTypeIsWider() {
  ARE_BOTH_SIGNED_OR_UNSIGNED(FirstType, SecondType);
  if constexpr (getTypeSizeInBits<FirstType>() >=
                getTypeSizeInBits<SecondType>()) {
    static_assert(std::numeric_limits<FirstType>::max() >=
                  std::numeric_limits<SecondType>::max());
    static_assert(std::numeric_limits<FirstType>::lowest() <=
                  std::numeric_limits<SecondType>::lowest());
    return true;
  } else {
    static_assert(std::numeric_limits<FirstType>::max() <=
                  std::numeric_limits<SecondType>::max());
    static_assert(std::numeric_limits<FirstType>::lowest() >=
                  std::numeric_limits<SecondType>::lowest());
    return false;
  }
}

template <typename T>
constexpr bool checkIfTypeHasConvRankGeqThanInt() {
  /* searching for better way to get conversion rank */
  return sizeof(T) >= sizeof(int);
}

struct UnusedArgs final {
  template <typename... Args>
  UnusedArgs(Args const&...) {}
};

namespace tmp_functions {

/* Works only for narrow range of types!
 * Better way is to use AST method getTypeAsString() of QualType
 * or to use preprocessor; but in some cases both ways can't to be used.
 * In future this function based on specializations will be replaced by typeid
 * from boost */
template <typename T>
constexpr const char* getIntTypeAsString() {
  return "undefined type";
}
template <>
constexpr const char* getIntTypeAsString<int>() {
  return "int";
}
template <>
constexpr const char* getIntTypeAsString<unsigned int>() {
  return "unsigned int";
}
template <>
constexpr const char* getIntTypeAsString<long>() {
  return "long";
}
template <>
constexpr const char* getIntTypeAsString<unsigned long>() {
  return "unsigned long";
}
template <>
constexpr const char* getIntTypeAsString<long long>() {
  return "long long";
}
#ifdef __GNUC__
template <>
constexpr const char* getIntTypeAsString<__int128>() {
  return "__int128";
}
#endif

} // namespace tmp_functions
} // namespace arithm_ut
} // namespace ub_tester
