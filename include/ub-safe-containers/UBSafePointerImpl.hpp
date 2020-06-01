#pragma once

namespace ub_tester {

template <typename T>
UBSafePointer<T>::UBSafePointer(std::nullptr_t) : Inited_{true} {}

template <typename T>
UBSafePointer<T>::UBSafePointer(T* Data, size_t Size)
    : Data_{Data}, Size_{Size}, Inited_{true} {}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::operator=(T* Data) {
  Data_ = Data;
}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::operator+=(int Val) {
  Data_ += Val;
  return *this;
}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::operator++() {
  ++Data_;
  return *this;
}

template <typename T>
UBSafePointer<T> UBSafePointer<T>::operator++(int) {
  UBSafePointer<T> Copy{*this};
  ++Data_;
  return Copy;
}

template <typename T>
const T& UBSafePointer<T>::operator*() const {
  return *Data_;
}

template <typename T>
T& UBSafePointer<T>::operator*() {
  return *Data_;
}

template <typename T>
const T& UBSafePointer<T>::operator->() const {
  return *Data_;
}

template <typename T>
T& UBSafePointer<T>::operator->() {
  return *Data_;
}

template <typename T>
const T& UBSafePointer<T>::operator[](int Val) const {
  return Data_[Val];
}

template <typename T>
T& UBSafePointer<T>::operator[](int Val) {
  return Data_[Val];
}

template <typename T>
UBSafePointer<T>::operator T*() {
  return Data_;
}

template <typename T>
UBSafePointer<T>& UBSafePointer<T>::setSize(size_t NewSize) {
  Size_ = NewSize;
  return *this;
}

template <typename T>
size_t UBSafePointer<T>::getSize() const {
  return Size_;
}
template <typename T>
PtrStateKind UBSafePointer<T>::getState() const {
  if (!Inited_)
    return PtrStateKind::Uninit;
  if (Data_ == nullptr)
    return PtrStateKind::Nullptr;
  return PtrStateKind::Init;
}

template <typename T>
UBSafePointer<T> operator+(const UBSafePointer<T>& SafePtr, int Val) {
  UBSafePointer<T> Res{SafePtr};
  Res += Val;
  return Res;
}

template <typename T>
UBSafePointer<T> operator+(int Val, const UBSafePointer<T>& SafePtr) {
  UBSafePointer<T> Res{SafePtr};
  Res += Val;
  return Res;
}

} // namespace ub_tester
