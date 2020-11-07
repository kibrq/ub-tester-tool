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

#define ASSERT_IOB(Lhs, Rhs)                                                   \
  ub_tester::carr_ptr::checkers::checkIOB((Lhs), (Rhs), __FILE__, __LINE__)

#define ASSERT_INVALID_SIZE(Sizes)                                             \
  ub_tester::carr_ptr::checkers::checkInvalidSize((Sizes), __FILE__, __LINE__)

#define ASSERT_STAROPERATOR(Pointer)                                           \
  ub_tester::carr_ptr::checkers::checkStarOperator((Pointer), __FILE__, __LINE__)
  
#define ASSERT_MEMBEREXPR(Pointer, Member)                                     \
  ub_tester::carr_ptr::checkers::checkMemberExpr((Pointer), __FILE__, __LINE__)  \
      ->(Member)

namespace ub_tester::carr_ptr::checkers {

using namespace ub_safe_carray;
using namespace ub_safe_ptr;

using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;

namespace {

inline void generateAssertIOBMessage(const char* Filename, size_t Line,
                                     int Index, size_t Size) {
  std::stringstream SStream;
  SStream << "Index out of "
             "bounds! "
          << " in file " << Filename << " on line " << Line << ".\n";
  SStream << "Requesting index " << Index << ", while size is " << Size << '\n';
  PUSH_ERROR(INDEX_OUT_OF_BOUNDS_ERROR, SStream.str());
}

inline void generateAssertInvalidSizeMessage(const char* Filename, size_t Line,
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

inline void generateAssertNullptrDeref(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Nullptr derefing "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_ERROR(NULLPTR_DEREF_ERROR, SStream.str());
}

inline void generateAssertUninitDeref(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Uninit pointer derefing "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_ERROR(UNINIT_PTR_DEREF_ERROR, SStream.str());
}

inline void generateUntrackedPtrMessage(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Untracked pointer "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_WARNING(UNTRACKED_PTR_WARNING, SStream.str());
}

template <typename T>
void handlePointer(const UBSafePointer<T>& Pointer, const char* Filename,
                   size_t Line) {
  switch (Pointer.getState()) {
  case PtrStateKind::Uninit:
    generateAssertUninitDeref(Filename, Line);
    break;
  case PtrStateKind::Nullptr:
    generateAssertNullptrDeref(Filename, Line);
    break;
  default:
    return;
  }
}

} // namespace

template <typename T, size_t N>
T& checkIOB(UBSafeCArray<T, N>& SafeArray, int Index, const char* Filename,
            size_t Line) {
  if (Index < 0 || Index >= SafeArray.getSize())
    generateAssertIOBMessage(Filename, Line, Index, SafeArray.getSize());
  return SafeArray[Index];
}

template <typename T, size_t N>
const T& checkIOB(const UBSafeCArray<T, N>& SafeArray, int Index,
                  const char* Filename, size_t Line) {
  if (Index < 0 || Index >= SafeArray.size())
    generateAssertIOBMessage(Filename, Line, Index, SafeArray.size());
  return SafeArray[Index];
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

inline std::vector<size_t> checkInvalidSize(const std::vector<int>& Sizes,
                                            const char* Filename, size_t Line) {
  std::vector<size_t> Res;
  for (const auto& Size : Sizes)
    if (Size < 0)
      generateAssertInvalidSizeMessage(Filename, Line, Size);
    else
      Res.emplace_back(Size);
  return Res;
}

template <typename T>
T& checkStarOperator(UBSafePointer<T>& Pointer, const char* Filename,
                     size_t Line) {

  handlePointer(Pointer, Filename, Line);

  return *Pointer;
}

template <typename T>
const T& checkStarOperator(const UBSafePointer<T>& Pointer,
                           const char* Filename, size_t Line) {
  handlePointer(Pointer, Filename, Line);
  return *Pointer;
}

template <typename T>
T& checkStarOperator(T*& Pointer, const char* Filename, size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return *Pointer;
}

template <typename T>
const T& checkStarOperator(const T*& Pointer, const char* Filename,
                           size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return *Pointer;
}

template <typename T>
UBSafePointer<T>& checkMemberExpr(UBSafePointer<T>& Pointer,
                                  const char* Filename, size_t Line) {
  handlePointer(Pointer, Filename, Line);
  return Pointer;
}

template <typename T>
const UBSafePointer<T>& checkMemberExpr(const UBSafePointer<T>& Pointer,
                                        const char* Filename, size_t Line) {
  handlePointer(Pointer, Filename, Line);
  return Pointer;
}

template <typename T>
T*& checkMemberExpr(T*& Pointer, const char* Filename, size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return Pointer;
}

template <typename T>
const T*& checkMemberExpr(const T*& Pointer, const char* Filename,
                          size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return Pointer;
}

template <typename T>
T& checkIOB(UBSafePointer<T>& Pointer, int Index, const char* Filename,
            size_t Line) {
  handlePointer(Pointer, Filename, Line);
  if (Index < 0 || Index >= Pointer.getSize())
    generateAssertIOBMessage(Filename, Line, Index, Pointer.getSize());
  return Pointer[Index];
}

template <typename T>
const T& checkIOB(const UBSafePointer<T>& Pointer, int Index,
                  const char* Filename, size_t Line) {
  handlePointer(Pointer, Filename, Line);
  if (Index < 0 || Index >= Pointer.getSize()) {
    generateAssertIOBMessage(Filename, Line, Index, Pointer.getSize());
  }
  return Pointer[Index];
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
  generateUntrackedPtrMessage(Filename, Line);
  return checkIOB(Array, Index, Filename, Line);
}

template <typename T>
const T& checkIOB(int Index, const T*& Array, const char* Filename,
                  size_t Line) {
  generateUntrackedPtrMessage(Filename, Line);
  return checkIOB(Array, Index, Filename, Line);
}

} // namespace ub_tester::carr_ptr::checkers
