#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace ub_tester {

template <typename T>
class UBSafeCArray {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<T>&);
  UBSafeCArray(const std::vector<size_t>& Sizes, bool);
  UBSafeCArray(
      const std::vector<size_t> Sizes, const std::initializer_list<T>&);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

  T& operator[](int index);
  const T& operator[](int index) const;

private:
  std::vector<T> Data_;
};

template <typename T>
class UBSafeCArray<UBSafeCArray<T>> {
public:
  explicit UBSafeCArray() = default;
  UBSafeCArray(const std::initializer_list<UBSafeCArray<T>>&);
  UBSafeCArray(const std::vector<size_t>& Sizes, bool);
  UBSafeCArray(
      const std::vector<size_t> Sizes,
      const std::initializer_list<UBSafeCArray<T>>&);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

  UBSafeCArray<T>& operator[](int index);
  const UBSafeCArray<T>& operator[](int index) const;

private:
  std::vector<UBSafeCArray<T>> Data_;
}; // namespace ub_tester

} // namespace ub_tester

#include "UBSafeCArrayImpl.hpp"
