#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace ub_tester {

template <typename T>
class UBSafeCArray {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<T>& args);

  void setSize(size_t Size);

  T& operator[](int index) noexcept;
  const T& operator[](int index) const noexcept;

private:
  std::vector<T> Data_;
};

template <>
class UBSafeCArray<char> {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<char>& args);
  UBSafeCArray(const char* str);

  void setSize(size_t Size);

  char& operator[](int index) noexcept;
  const char& operator[](int index) const noexcept;

private:
  std::vector<char> Data_;
};

} // namespace ub_tester

#include "UBSafeCArrayImpl.hpp"
