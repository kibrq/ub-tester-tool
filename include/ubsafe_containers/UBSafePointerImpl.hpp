#pragma once
#include <algorithm>

namespace ub_tester {

template <typename T>
UBSafePointer<T>::UBSafePointer() : data_(nullptr), size_(0) {}

template <typename T>
UBSafePointer<T>::UBSafePointer(const T*& data) : data_(data), size_(0) {}

template <typename T>
UBSafePointer<T>::UBSafePointer(T*&& data) : data_(data), size_(0) {
  data = nullptr;
}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::operator=(const T*& data) noexcept {
  data_ = data;
  return *this;
}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::operator=(T*&& data) noexcept {
  data_ = data;
  data = nullptr;
  return *this;
}

template <typename T>
T& UBSafePointer<T>::operator*() noexcept {
  if (data_)
    return *data_;
  else
    exit(0);
}

template <typename T>
T* UBSafePointer<T>::operator->() noexcept {
  if (data_)
    return data_;
  else
    exit(0);
}

template <typename T>
T& UBSafePointer<T>::operator[](int index) noexcept {
  if (index >= size_) {
    exit(0);
  }
  return data_[index];
}

template <typename T>

const T& UBSafePointer<T>::operator[](int index) const noexcept {
  if (index >= size_) {
    exit(0);
  }
  return data_[index];
}

template <typename T>
UBSafePointer<T>::operator T*() noexcept {
  return data_;
}

template <typename T>
UBSafePointer<T>::operator bool() noexcept {
  return data_ == nullptr;
}

template <typename T>
void UBSafePointer<T>::setSize(size_t size) noexcept {
  size_ = size;
}

template <typename T>
void UBSafePointer<T>::setRawData(T* data) noexcept {
  data_ = data;
  size_ = 0;
}

template <typename T>
size_t UBSafePointer<T>::getSize() const noexcept {
  return size_;
}

} // namespace ub_tester
