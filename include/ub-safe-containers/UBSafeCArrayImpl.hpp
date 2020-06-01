#pragma once

#include <cassert>
#include <cstring>

namespace ub_tester::ub_safe_carray {

template <typename T, size_t N>
UBSafeCArray<T, N>::UBSafeCArray() {
  Data_.resize(N);
}

template <typename T, size_t N>
UBSafeCArray<T, N>::UBSafeCArray(const std::initializer_list<T>& InitList)
    : Data_{InitList} {
  if (N != 0)
    setSize(N);
}

template <typename T, size_t N>
void UBSafeCArray<T, N>::setSize(size_t Size) {
  Data_.resize(Size);
}

template <typename T, size_t N>
void UBSafeCArray<T, N>::setSize(const std::vector<size_t>& Sizes,
                                 int CurDepth) {
  setSize(Sizes[CurDepth]);
}

template <typename T, size_t N>
size_t UBSafeCArray<T, N>::getSize() const {
  return Data_.size();
}

template <typename T, size_t N>
UBSafeCArray<T, N>::UBSafeCArray(const std::vector<size_t>& Sizes) {
  setSize(Sizes);
}

template <typename T, size_t N>
UBSafeCArray<T, N>::UBSafeCArray(std::vector<size_t> Sizes,
                                 const std::initializer_list<T>& Args)
    : UBSafeCArray(Args) {
  setSize(Sizes);
}

template <typename T, size_t N>
const T& UBSafeCArray<T, N>::operator[](int Index) const {
  return Data_.at(Index);
}

template <typename T, size_t N>
T& UBSafeCArray<T, N>::operator[](int Index) {
  return Data_.at(Index);
}

// Multi-dimensional specialization

template <typename T, size_t N, size_t M>
UBSafeCArray<UBSafeCArray<T, N>, M>::UBSafeCArray() {
  Data_.resize(M);
}

template <typename T, size_t N, size_t M>
UBSafeCArray<UBSafeCArray<T, N>, M>::UBSafeCArray(
    const std::initializer_list<UBSafeCArray<T, N>>& InitList)
    : Data_{InitList} {
  if (M != 0)
    setSize(M);
}

template <typename T, size_t N, size_t M>
void UBSafeCArray<UBSafeCArray<T, N>, M>::setSize(size_t Size) {
  Data_.resize(Size);
}

template <typename T, size_t N, size_t M>
void UBSafeCArray<UBSafeCArray<T, N>, M>::setSize(
    const std::vector<size_t>& Sizes, int CurDepth) {
  setSize(Sizes[CurDepth]);
  for (auto& Arr : Data_)
    Arr.setSize(Sizes, CurDepth + 1);
}

template <typename T, size_t N, size_t M>
size_t UBSafeCArray<UBSafeCArray<T, N>, M>::getSize() const {
  return Data_.size();
}

template <typename T, size_t N, size_t M>
UBSafeCArray<UBSafeCArray<T, N>, M>::UBSafeCArray(
    const std::vector<size_t>& Sizes) {
  setSize(Sizes);
}

template <typename T, size_t N, size_t M>
UBSafeCArray<UBSafeCArray<T, N>, M>::UBSafeCArray(
    std::vector<size_t> Sizes,
    const std::initializer_list<UBSafeCArray<T, N>>& Args)
    : Data_{Args} {
  setSize(Sizes);
}

template <typename T, size_t N, size_t M>
const UBSafeCArray<T, N>&
UBSafeCArray<UBSafeCArray<T, N>, M>::operator[](int Index) const {
  return Data_.at(Index);
}

template <typename T, size_t N, size_t M>
UBSafeCArray<T, N>& UBSafeCArray<UBSafeCArray<T, N>, M>::operator[](int Index) {
  return Data_.at(Index);
}

// char specialization

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray() {
  Data_.resize(N);
}

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray(const std::initializer_list<char>& InitList)
    : Data_{InitList} {
  if (N != 0)
    setSize(N);
}

template <size_t N>
void UBSafeCArray<char, N>::setSize(size_t Size) {
  Data_.resize(Size);
}

template <size_t N>
void UBSafeCArray<char, N>::setSize(const std::vector<size_t>& Sizes,
                                    int CurDepth) {
  setSize(Sizes[CurDepth]);
}

template <size_t N>
size_t UBSafeCArray<char, N>::getSize() const {
  return Data_.size();
}

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray(const std::vector<size_t>& Sizes) {
  setSize(Sizes);
}

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray(std::vector<size_t> Sizes,
                                    const std::initializer_list<char>& Args)
    : UBSafeCArray(Args) {
  setSize(Sizes);
}

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray(std::vector<size_t> Sizes,
                                    const char* StringLiteral)
    : UBSafeCArray(StringLiteral) {
  setSize(Sizes);
}

template <size_t N>
UBSafeCArray<char, N>::UBSafeCArray(const char* StringLiteral) {
  setSize(strlen(StringLiteral) + 1);
  size_t i = 0;
  while (*StringLiteral != '\0')
    Data_[i++] = *StringLiteral;
}

template <size_t N>
const char& UBSafeCArray<char, N>::operator[](int Index) const {
  return Data_.at(Index);
}

template <size_t N>
char& UBSafeCArray<char, N>::operator[](int Index) {
  return Data_.at(Index);
}

} // namespace ub_tester::ub_safe_carray
