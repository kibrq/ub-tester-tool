#pragma once
#include <cassert>

namespace ub_tester {

template <typename T>
UBSafeCArray<T>::UBSafeCArray(const std::initializer_list<T>& InitList)
    : Data_{InitList} {}

template <typename T>
void UBSafeCArray<T>::setSize(size_t Size) {
  Data_.resize(Size);
}

template <typename T>
void UBSafeCArray<T>::setSize(const std::vector<size_t>& Sizes, int CurDepth) {
  setSize(Sizes[CurDepth]);
}

template <typename T>
UBSafeCArray<T>::UBSafeCArray(const std::vector<size_t>& Sizes, bool) {
  setSize(Sizes);
}

template <typename T>
UBSafeCArray<T>::UBSafeCArray(
    std::vector<size_t> Sizes, const std::initializer_list<T>& Args)
    : UBSafeCArray(Sizes, false) {
  Data_ = Args;
}

template <typename T>
const T& UBSafeCArray<T>::operator[](int Index) const {
  return Data_.at(Index);
}

template <typename T>
T& UBSafeCArray<T>::operator[](int Index) {
  return Data_.at(Index);
}

// UBSafeCArray specialization

template <typename T>
UBSafeCArray<UBSafeCArray<T>>::UBSafeCArray(
    const std::initializer_list<UBSafeCArray<T>>& InitList)
    : Data_{InitList} {}

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
UBSafeCArray<UBSafeCArray<T>>::UBSafeCArray(
    const std::vector<size_t>& Sizes, bool) {
  setSize(Sizes);
}

template <typename T>
UBSafeCArray<UBSafeCArray<T>>::UBSafeCArray(
    std::vector<size_t> Sizes,
    const std::initializer_list<UBSafeCArray<T>>& Args)
    : Data_{Args} {
  setSize(Sizes);
}

template <typename T>
const UBSafeCArray<T>&
UBSafeCArray<UBSafeCArray<T>>::operator[](int Index) const {
  return Data_.at(Index);
}

template <typename T>
UBSafeCArray<T>& UBSafeCArray<UBSafeCArray<T>>::operator[](int Index) {
  return Data_.at(Index);
}

} // namespace ub_tester
