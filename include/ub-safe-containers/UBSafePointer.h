#pragma once

#include <cstddef>

namespace ub_tester {

template <typename T>
class UBSafePointer {
public:
  UBSafePointer() = default;
  UBSafePointer(nullptr_t);
  explicit UBSafePointer(T* Data, size_t Size = 1);

  UBSafePointer<T>& operator=(T* Data);
  UBSafePointer<T>& operator+=(int Val);
  UBSafePointer<T>& operator++();
  UBSafePointer<T> operator++(int);

  const T& operator*() const;
  T& operator*();
  const T& operator->() const;
  T& operator->();
  const T& operator[](int Val) const;
  T& operator[](int Val);

  operator T*();

  UBSafePointer<T>& setSize(size_t NewSize);

private:
  T* Data_{nullptr};
  size_t Size_{0};
  bool Inited_{false};
};
template <typename T>
UBSafePointer<T> operator+(const UBSafePointer<T>& P, int Val);

template <typename T>
UBSafePointer<T> operator+(int Val, const UBSafePointer<T>& P);

#include "UBSafePointerImpl.hpp"
} // namespace ub_tester
