#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace ub_tester::ub_safe_carray {

template <typename T, size_t N = 0>
class UBSafeCArray {
public:
  explicit UBSafeCArray();
  UBSafeCArray(const std::initializer_list<T>&);
  UBSafeCArray(const std::vector<size_t>& Sizes);
  UBSafeCArray(const std::vector<size_t> Sizes,
               const std::initializer_list<T>&);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);

  size_t getSize() const;

  T& operator[](int index);
  const T& operator[](int index) const;

private:
  std::vector<T> Data_;
};

// Multi-dimensional specialization

template <typename T, size_t N, size_t M>
class UBSafeCArray<UBSafeCArray<T, N>, M> {
public:
  explicit UBSafeCArray();
  UBSafeCArray(const std::initializer_list<UBSafeCArray<T, N>>&);
  UBSafeCArray(const std::vector<size_t>& Sizes);
  UBSafeCArray(const std::vector<size_t> Sizes,
               const std::initializer_list<UBSafeCArray<T, N>>&);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);
  size_t getSize() const;

  UBSafeCArray<T, N>& operator[](int index);
  const UBSafeCArray<T, N>& operator[](int index) const;

private:
  std::vector<UBSafeCArray<T, N>> Data_;
};

// char specialization

template <size_t N>
class UBSafeCArray<char, N> {
public:
  explicit UBSafeCArray();
  UBSafeCArray(const char* StringLiteral);
  UBSafeCArray(const std::initializer_list<char>&);
  UBSafeCArray(const std::vector<size_t>& Sizes);
  UBSafeCArray(const std::vector<size_t> Sizes,
               const std::initializer_list<char>&);
  UBSafeCArray(const std::vector<size_t> Sizes, const char* StringLiteral);

  void setSize(size_t Size);
  void setSize(const std::vector<size_t>& Sizes, int CurDepth = 0);
  size_t getSize() const;

  char& operator[](int index);
  const char& operator[](int index) const;

private:
  std::vector<char> Data_;
};

} // namespace ub_tester::ub_safe_carray

#include "UBSafeCArrayImpl.hpp"
