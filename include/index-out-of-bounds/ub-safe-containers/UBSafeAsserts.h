#pragma once

#include "UBSafeCArray.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace ub_tester {

#define ASSERT_INDEX_OUT_OF_BOUNDS(Lhs, Rhs)                                   \
  ub_tester::checkIOB(Lhs, Rhs, __FILE__, __LINE__)

#define ASSERT_INVALID_SIZE(Sizes)                                             \
  ub_tester::checkInvalidSize(Sizes, __FILE__, __LINE__)

template <typename T>
T& checkIOB(UBSafeCArray<T>& Array, int Index, const char* Filename,
            size_t LineNum) {
  try {
    return Array[Index];
  } catch (const std::out_of_range& e) {
    std::cout << "Index out of bounds!" << std::endl
              << "On " << Filename << "::" << LineNum << std::endl;
    std::abort();
  }
}

template <typename T>
const T& checkIOB(const UBSafeCArray<T>& Array, int Index, const char* Filename,
                  size_t LineNum) {
  try {
    return Array[Index];
  } catch (const std::out_of_range& e) {
    std::cout << "Index out of bounds!" << std::endl
              << "On " << Filename << "::" << LineNum << std::endl;
    std::abort();
  }
}

template <typename T>
T& checkIOB(int Index, UBSafeCArray<T>& Array, const char* Filename,
            size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

template <typename T>
const T& checkIOB(int Index, const UBSafeCArray<T>& Array, const char* Filename,
                  size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

template <typename T>
T& checkIOB(T*& Array, int Index, const char* Filename, size_t LineNum) {
  // TODO
  return Array[Index];
}

template <typename T>
const T& checkIOB(const T*& Array, int Index, const char* Filename,
                  size_t LineNum) {
  // TODO
  return Array[Index];
}

template <typename T>
T& checkIOB(int Index, T*& Array, const char* Filename, size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

template <typename T>
const T& checkIOB(int Index, const T*& Array, const char* Filename,
                  size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

template <typename T>
T& checkIOB(T Array[], int Index, const char* Filename, size_t LineNum) {
  return Array[Index];
}

template <typename T>
const T& checkIOB(const T Array[], int Index, const char* Filename,
                  size_t LineNum) {
  return Array[Index];
}

template <typename T>
T& checkIOB(int Index, T Array[], const char* Filename, size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

template <typename T>
const T& checkIOB(int Index, const T Array[], const char* Filename,
                  size_t LineNum) {
  return checkIOB(Array, Index, Filename, LineNum);
}

inline std::vector<size_t> checkInvalidSize(const std::vector<int>& Sizes,
                                            const char* Filename,
                                            size_t LineNum) {
  std::vector<size_t> Res;
  for (const auto& Size : Sizes) {
    if (Size < 0 || Size >= SIZE_MAX) {
      std::cout << "Invalid size of an array" << std::endl
                << "On " << Filename << "::" << LineNum << std::endl;
      std::abort();
    } else {
      Res.emplace_back(Size);
    }
  }
  return Res;
}
} // namespace ub_tester
