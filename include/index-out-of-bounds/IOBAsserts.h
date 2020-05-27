#pragma once

#include "../ub-safe-containers/UBSafeCArray.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ub_tester::iob::checkers {

#define ASSERT_IOB(Lhs, Rhs)                                                   \
  ub_tester::iob::checkers::checkIOB(                                          \
      Lhs, Rhs,                                                                \
      ub_tester::iob::checkers::IOBCheckerHelper(#Lhs, Rhs, __FILE__,          \
                                                 __LINE__))

#define ASSERT_INVALID_SIZE(Sizes)                                             \
  ub_tester::iob::checkers::checkInvalidSize(                                  \
      Sizes,                                                                   \
      ub_tester::iob::checkers::InvalidSizeCheckerHelper(__FILE__, __LINE__))

struct IOBCheckerHelper {
  IOBCheckerHelper(const char* VarName, int Index, const char* Filename,
                   size_t LineNum)
      : VarName_{VarName}, Index_{Index}, Filename_{Filename},
        LineNum_{LineNum}, Size_{0} {}

  void setSize(size_t Size) { Size_ = Size; }

  void signalizeError() const {
    std::stringstream Message;
    Message << "In " << Filename_ << " on line " << LineNum_
            << " Index Out Of Bounds!!\n"
            << "Size of variable "
            << "\'" << VarName_ << "\'"
            << " is " << Size_ << " but requested index is " << Index_ << '\n';
    std::cout << Message.str();
    exit(1);
  }

  void signalizeWarning() const {}

private:
  std::string VarName_;
  int Index_;
  std::string Filename_;
  size_t LineNum_;
  size_t Size_;
};

template <typename T>
T& checkIOB(UBSafeCArray<T>& Array, int Index, IOBCheckerHelper Helper) {
  try {
    return Array[Index];
  } catch (const std::out_of_range& e) {
    Helper.setSize(Array.getSize());
    Helper.signalizeError();
  }
}

template <typename T>
const T& checkIOB(const UBSafeCArray<T>& Array, int Index,
                  IOBCheckerHelper Helper) {
  try {
    return Array[Index];
  } catch (const std::out_of_range& e) {
    Helper.setSize(Array.getSize());
    Helper.signalizeError();
  }
}

template <typename T>
T& checkIOB(T*& Array, int Index, IOBCheckerHelper Helper) {
  Helper.signalizeWarning();
  return Array[Index];
}

template <typename T>
const T& checkIOB(const T*& Array, int Index, IOBCheckerHelper Helper) {
  Helper.signalizeWarning();
  return Array[Index];
}

template <typename T>
T& checkIOB(int Index, T*& Array, IOBCheckerHelper Helper) {
  return checkIOB(Array, Index, Helper);
}

template <typename T>
const T& checkIOB(int Index, const T*& Array, IOBCheckerHelper Helper) {
  return checkIOB(Array, Index, Helper);
}

template <typename T, size_t N>
T& checkIOB(T (&Array)[N], int Index, IOBCheckerHelper Helper) {
  if (Index < 0 || Index >= N) {
    Helper.setSize(N);
    Helper.signalizeError();
  }
  return Array[Index];
}

template <typename T, size_t N>
const T& checkIOB(const T (&Array)[N], int Index, IOBCheckerHelper Helper) {
  if (Index < 0 || Index >= N) {
    Helper.setSize(N);
    Helper.signalizeError();
  }
  return Array[Index];
}

template <typename T, size_t N>
T& checkIOB(int Index, T (&Array)[N], IOBCheckerHelper Helper) {
  return checkIOB<T, N>(Array, Index, Helper);
}

template <typename T, size_t N>
const T& checkIOB(int Index, const T (&Array)[N], IOBCheckerHelper Helper) {
  return checkIOB<T, N>(Array, Index, Helper);
}

struct InvalidSizeCheckerHelper {
  InvalidSizeCheckerHelper(const char* Filename, size_t LineNum)
      : Filename_{Filename}, LineNum_{LineNum}, InvalidSizeValue_{0} {}

  void setInvalidSize(int InvalidSize) { InvalidSizeValue_ = InvalidSize; }

  void signalizeError() const {
    std::stringstream Message;
    Message << "In " << Filename_ << " on line " << LineNum_
            << " Invalid size of an array!!\n"
            << "Trying to make ARRAY of size " << InvalidSizeValue_ << '\n';
    std::cout << Message.str();
    exit(1);
  }

  void signalizeWarning() const {}

private:
  std::string Filename_;
  size_t LineNum_;
  int InvalidSizeValue_;
};

inline std::vector<size_t> checkInvalidSize(const std::vector<int>& Sizes,
                                            InvalidSizeCheckerHelper Helper) {
  std::vector<size_t> Res;
  for (const auto& Size : Sizes) {
    if (Size < 0 || Size >= SIZE_MAX) {
      Helper.setInvalidSize(Size);
      Helper.signalizeError();
    } else {
      Res.emplace_back(Size);
    }
  }
  return Res;
}
} // namespace ub_tester::iob::checkers
