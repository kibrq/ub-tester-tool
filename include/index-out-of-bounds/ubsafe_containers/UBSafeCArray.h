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
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

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
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

  char& operator[](int index) noexcept;
  const char& operator[](int index) const noexcept;

private:
  std::vector<char> Data_;
};

template <typename T>
class UBSafeCArray<UBSafeCArray<T>> {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<UBSafeCArray<T>>& args);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

  UBSafeCArray<T>& operator[](int index) noexcept;
  const UBSafeCArray<T>& operator[](int index) const noexcept;

private:
  std::vector<UBSafeCArray<T>> Data_;
};
} // namespace ub_tester

#include "UBSafeCArrayImpl.hpp"
