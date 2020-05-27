#ifndef UB_POINTERS_ASSERTS_INCLUDE_H_
#define UB_POINTERS_ASSERTS_INCLUDE_H_

#include "../assert-message-manager/AssertMessageManager.h"
#include "UBSafePointer.h"

namespace ub_tester::ptr::checkers {

using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;

#define ASSERT_STAROPERATOR(Pointer)                                           \
  ptr::checkers::checkStarOperator(Pointer, __FILE__, __LINE__)
#define ASSERT_MEMBEREXPR(Pointer, Member)                                     \
  ptr::checkers::checkMemberExpr(Pointer, __FILE__, __LINE__)->Member

namespace {
void generateAssertNullptrDered(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Nullptr derefing "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_ERROR(NULLPTR_DEREF_ERROR, SStream.str());
}
void generateUntrackedPtrMessage(const char* Filename, size_t Line) {
  std::stringstream SStream;
  SStream << "Untracked pointer "
          << " in file " << Filename << " on line " << Line << ".\n";
  PUSH_WARNING(UNTRACKED_PTR_WARNING, SStream.str());
}

template <typename T>
void handlePointer(const UBSafePointer<T>& Pointer, const char* Filename,
                   size_t LineNum) {
  switch (Pointer.getState()) {
  case PtrStateKind::Uninit:
    generateAssertNullptrDered(Filename, LineNum);
    break;
  case PtrStateKind::Nullptr:
    generateAssertNullptrDered(Filename, LineNum);
    break;
  default:
    return;
  }
}
} // namespace

template <typename T>
T& checkStarOperator(UBSafePointer<T>& Pointer, const char* Filename,
                     size_t LineNum) {

  handlePointer(Pointer, Filename, LineNum);

  return *Pointer;
}

template <typename T>
const T& checkStarOperator(const UBSafePointer<T>& Pointer,
                           const char* Filename, size_t LineNum) {
  handlePointer(Pointer, Filename, LineNum);
  return *Pointer;
}

template <typename T>
T& checkStarOperator(T*& Pointer, const char* Filename, size_t LineNum) {
  generateUntrackedPtrMessage(Filename, LineNum);
  return *Pointer;
}

template <typename T>
const T& checkStarOperator(const T*& Pointer, const char* Filename,
                           size_t LineNum) {
  generateUntrackedPtrMessage(Filename, LineNum);
  return *Pointer;
}

template <typename T>
UBSafePointer<T>& checkMemberExpr(UBSafePointer<T>& Pointer,
                                  const char* Filename, size_t LineNum) {
  handlePointer(Pointer, Filename, LineNum);
  return Pointer;
}

template <typename T>
const UBSafePointer<T>& checkMemberExpr(const UBSafePointer<T>& Pointer,
                                        const char* Filename, size_t LineNum) {
  handlePointer(Pointer, Filename, LineNum);
  return Pointer;
}

template <typename T>
T*& checkMemberExpr(T*& Pointer, const char* Filename, size_t LineNum) {
  generateUntrackedPtrMessage(Filename, LineNum);
  return Pointer;
}

template <typename T>
const T*& checkMemberExpr(const T*& Pointer, const char* Filename,
                          size_t LineNum) {
  generateUntrackedPtrMessage(Filename, LineNum);
  return Pointer;
}

} // namespace ub_tester::ptr::checkers

#endif
