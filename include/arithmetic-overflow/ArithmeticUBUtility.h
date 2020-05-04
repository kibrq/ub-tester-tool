#pragma once

#include <cassert>
#include <climits>
#include <cstddef>
#include <limits>

#define ARE_BOTH_SIGNED_OR_UNSIGNED(Type1, Type2)                              \
  assert((std::numeric_limits<Type1>::is_signed ==                             \
          std::numeric_limits<Type2>::is_signed))

namespace ub_tester {
namespace arithm_ut {

template <typename T>
size_t getTypeSizeInBits() {
  return sizeof(T) * CHAR_BIT;
}

template <typename FirstType, typename SecondType>
bool checkIfFirstTypeIsWider() {
  ARE_BOTH_SIGNED_OR_UNSIGNED(FirstType, SecondType);
  if (getTypeSizeInBits<FirstType>() >= getTypeSizeInBits<SecondType>()) {
    assert((std::numeric_limits<FirstType>::max() >=
            std::numeric_limits<SecondType>::max()));
    assert((std::numeric_limits<FirstType>::lowest() <=
            std::numeric_limits<SecondType>::lowest()));
    return true;
  }
  assert((std::numeric_limits<FirstType>::max() <=
          std::numeric_limits<SecondType>::max()));
  assert((std::numeric_limits<FirstType>::lowest() >=
          std::numeric_limits<SecondType>::lowest()));
  return false;
}

template <typename Type>
bool checkIfTypeHasConvRankGeqThanInt() {
  /* searching for better way to get conversion rank */
  return sizeof(Type) >= sizeof(int);
}

} // namespace arithm_ut
} // namespace ub_tester
