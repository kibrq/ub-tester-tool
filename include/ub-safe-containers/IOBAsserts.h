#pragma once

#include "../assert-message-manager/AssertMessageManager.h"
#include "UBSafeCArray.h"
#include "UBSafePointer.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ub_tester::iob::checkers {

using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;

#define ASSERT_IOB(Lhs, Rhs)                                                   \
  ub_tester::iob::checkers::checkIOB(Lhs, Rhs, __FILE__, __LINE__)

#define ASSERT_INVALID_SIZE(Sizes)                                             \
  ub_tester::iob::checkers::checkInvalidSize(Sizes, __FILE__, __LINE__)

namespace {

void generateAssertIOBMessage(const char* Filename, size_t Line, int Index,
                              size_t Size) {
  std::stringstream SStream;
  SStream << "Index out of "
             "bounds! "
          << " in file " << Filename << " on line " << Line << ".\n";
  SStream << "Requesting index " << Index << ", while size is " << Size << '\n';
  PUSH_ERROR(INDEX_OUT_OF_BOUNDS_ERROR, SStream.str());
}

void generateAssertInvalidSizeMessage(const char* Filename, size_t Line,
                                      int InvalidDim) {
  std::stringstream SStream;
  SStream << "Invalid size "
             "of an array! "
          << " in file " << Filename << " on line " << Line << ".\n";
  SStream << "Trying to create "
             "an array of size "
          << InvalidDim << '\n';
  PUSH_ERROR(INVALID_SIZE_OF_ARRAY_ERROR, SStream.str());
}

void generateUntrackedPtrMessage(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Untracked pointer "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_WARNING(UNTRACKED_PTR_WARNING, SStream.str());
}

} // namespace

template <typename T>
T& checkIOB(UBSafeCArray<T>& SafeArray, int Index, const char* Filename,
            size_t Line) {
  try {
    return SafeArray[Index];
  } catch (const std::out_of_range& e) {
    generateAssertIOBMessage(Filename, Line, Index, SafeArray.getSize());
  }
  return SafeArray[Index];
}

template <typename T>
const T& checkIOB(const UBSafeCArray<T>& SafeArray, int Index,
                  const char* Filename, size_t Line) {
  try {
    return SafeArray[Index];
  } catch (const std::out_of_range& e) {
    generateAssertIOBMessage(Filename, Line, Index, SafeArray.getSize());
  }
}

template <typename T>
T& checkIOB(T*& Array, int Index, const char* Filename, size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return Array[Index];
}

template <typename T>
const T& checkIOB(const T*& Array, int Index, const char* Filename,
                  size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return Array[Index];
}

template <typename T>
T& checkIOB(int Index, T*& Array, const char* Filename, size_t Line) {
  return checkIOB(Array, Index, Filename, Line);
}

template <typename T>
const T& checkIOB(int Index, const T*& Array, const char* Filename,
                  size_t Line) {
  return checkIOB(Array, Index, Filename, Line);
}

template <typename T, size_t N>
T& checkIOB(T (&Array)[N], int Index, const char* Filename, size_t Line) {
  if (Index < 0 || Index >= N)
    generateAssertIOBMessage(Filename, Line, Index, N);
  return Array[Index];
}

template <typename T, size_t N>
const T& checkIOB(const T (&Array)[N], int Index, const char* Filename,
                  size_t Line) {
  if (Index < 0 || Index >= N)
    generateAssertIOBMessage(Filename, Line, Index, N);
  return Array[Index];
}

template <typename T, size_t N>
T& checkIOB(int Index, T (&Array)[N], const char* Filename, size_t Line) {
  return checkIOB<T, N>(Array, Index, Filename, Line);
}

template <typename T, size_t N>
const T& checkIOB(int Index, const T (&Array)[N], const char* Filename,
                  size_t Line) {
  return checkIOB<T, N>(Array, Index, Filename, Line);
}

template <typename T>
T& checkIOB(UBSafePointer<T>& SafePtr, int Index, const char* Filename,
            size_t Line) {
  if (Index < 0 || Index >= SafePtr.getSize())
    generateAssertIOBMessage(Filename, Line, Index, SafePtr.getSize());
  return SafePtr[Index];
}

template <typename T>
const T& checkIOB(const UBSafePointer<T>& SafePtr, int Index,
                  const char* Filename, size_t Line) {
  if (Index < 0 || Index >= SafePtr.getSize()) {
    generateAssertIOBMessage(Filename, Line, Index, SafePtr.getSize());
  }
  return SafePtr[Index];
}

inline std::vector<size_t> checkInvalidSize(const std::vector<int>& Sizes,
                                            const char* Filename, size_t Line) {
  std::vector<size_t> Res;
  for (const auto& Size : Sizes)
    if (Size < 0 || Size >= SIZE_MAX)
      generateAssertInvalidSizeMessage(Filename, Line, Size);
    else
      Res.emplace_back(Size);
  return Res;
}

} // namespace ub_tester::iob::checkers
