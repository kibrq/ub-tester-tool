#pragma once

#include <cstddef>
#include <initializer_list>

namespace ub_tester {

template <typename T, size_t N>
class UBSafeCArray {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<T>& args);

  T& operator[](int index) noexcept;
  const T& operator[](int index) const noexcept;

private:
  T data_[N];
};

template <size_t N>
class UBSafeCArray<char, N> {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<char>& args);
  UBSafeCArray(const char* str);

  char& operator[](int index) noexcept;
  const char& operator[](int index) const noexcept;

private:
  char data_[N];
};

} // namespace ub_tester

#include "UBSafeCArrayImpl.hpp"
