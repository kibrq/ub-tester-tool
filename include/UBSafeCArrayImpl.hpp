#pragma once

namespace ub_tester {

template <typename T, size_t N>
T& UBSafeCArray<T, N>::operator[](int index) noexcept {
  if (index < 0 || index >= N) {
    // exit(0);
  }
  return data_[index];
}

template <typename T, size_t N>
const T& UBSafeCArray<T, N>::operator[](int index) const noexcept {
  if (index < 0 || index >= N) {
    // exit(0);
  }
  return data_[index];
}

template <typename T, size_t N>
UBSafeCArray<T, N>::UBSafeCArray(const std::initializer_list<T>& args) {
  size_t i = 0;
  for (const auto& el : args) {
    if (i == N)
      break;
    data_[i++] = el;
  }
}

template <size_t N>
char& UBSafeCArray<char, N>::operator[](int index) noexcept {
  if (index < 0 || index >= N) {
    // exit(0);
  }
  return data_[index];
}

template <size_t N>
const char& UBSafeCArray<char, N>::operator[](int index) const noexcept {
  if (index < 0 || index >= N) {
    // exit(0);
  }
  return data_[index];
}

template <std::size_t N>
UBSafeCArray<char, N>::UBSafeCArray(const std::initializer_list<char>& args) {
  size_t i = 0;
  for (const auto& el : args) {
    if (i == N)
      break;
    data_[i++] = el;
  }
}

template <std::size_t N>
UBSafeCArray<char, N>::UBSafeCArray(const char* str) {
  for (size_t i = 0; i < N; i++) {
    data_[i] = str[i];
  }
  data_[N - 1] = '\0';
}

} // namespace ub_tester
