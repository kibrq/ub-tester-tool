#pragma once

#include <cstddef>

namespace ub_tester::ub_safe_ptr {

enum PtrStateKind { Uninit, Nullptr, Init };

template <typename T>
class UBSafePointer {
public:
  UBSafePointer() = default;
  UBSafePointer(std::nullptr_t);
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
  size_t getSize() const;
  PtrStateKind getState() const;

private:
  T* Data_{nullptr};
  size_t Size_{0};
  bool Inited_{false};
};

template <typename T>
UBSafePointer<T> operator+(const UBSafePointer<T>& SafePtr, int Val);

template <typename T>
UBSafePointer<T> operator+(int Val, const UBSafePointer<T>& SafePtr);

} // namespace ub_tester::ub_safe_ptr

#include "UBSafePointerImpl.hpp"
