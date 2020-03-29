#pragma once
#include <cassert>

namespace ub_tester {

template <typename T>
void UBSafeCArray<T>::setSize(size_t Size) {
  Data_.resize(Size);
}

template <typename T>
void UBSafeCArray<T>::setSize(const std::vector<size_t>& Sizes, int CurDepth) {
  assert(CurDepth < Sizes.size());
  setSize(Sizes[CurDepth]);
}

template <typename T>
T& UBSafeCArray<T>::operator[](int index) noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

template <typename T>
const T& UBSafeCArray<T>::operator[](int index) const noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

template <typename T>
UBSafeCArray<T>::UBSafeCArray(const std::initializer_list<T>& args) {
  Data_ = args;
}

char& UBSafeCArray<char>::operator[](int index) noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

const char& UBSafeCArray<char>::operator[](int index) const noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

void UBSafeCArray<char>::setSize(size_t Size) { Data_.resize(Size); }
void UBSafeCArray<char>::setSize(
    const std::vector<size_t>& Sizes, int CurDepth) {
  setSize(Sizes[CurDepth]);
}

UBSafeCArray<char>::UBSafeCArray(const std::initializer_list<char>& args) {
  Data_ = args;
}

UBSafeCArray<char>::UBSafeCArray(const char* str) {
  do {
    Data_.push_back(*str);
  } while (*str != '\0');
}

template <typename T>
void UBSafeCArray<UBSafeCArray<T>>::setSize(size_t Size) {
  Data_.resize(Size);
}
template <typename T>
void UBSafeCArray<UBSafeCArray<T>>::setSize(
    const std::vector<size_t>& Sizes, int CurDepth) {
  setSize(Sizes[CurDepth]);
  for (auto& Arr : Data_) {
    Arr.setSize(Sizes, CurDepth + 1);
  }
}

template <typename T>
UBSafeCArray<T>& UBSafeCArray<UBSafeCArray<T>>::operator[](int index) noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

template <typename T>
const UBSafeCArray<T>&
UBSafeCArray<UBSafeCArray<T>>::operator[](int index) const noexcept {
  if (index < 0 || index >= Data_.size()) {
    // exit(0);
  }
  return Data_[index];
}

template <typename T>
UBSafeCArray<UBSafeCArray<T>>::UBSafeCArray(
    const std::initializer_list<UBSafeCArray<T>>& args) {
  Data_ = args;
}

} // namespace ub_tester
