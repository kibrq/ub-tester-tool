#pragma once

#include <cstddef>
#include <initializer_list>

namespace ub_tester {

template <typename T>
class UBSafePointer {
public:
  UBSafePointer();
  UBSafePointer(const T*&);
  UBSafePointer(T*&&);

  UBSafePointer& operator=(const T*&) noexcept;
  UBSafePointer& operator=(T*&&) noexcept;
  UBSafePointer& operator=(std::initializer_list<T> args) noexcept;

  T& operator*() noexcept;
  T* operator->() noexcept;

  T& operator[](int index) noexcept;
  const T& operator[](int index) const noexcept;

  operator T*() noexcept;

  operator bool() noexcept;

  void setSize(size_t size) noexcept;
  void setRawData(T* data) noexcept;
  size_t getSize() const noexcept;

private:
  T* data_;
  size_t size_;
};

}; // namespace ub_tester

#include "UBSafePointerImpl.hpp"
