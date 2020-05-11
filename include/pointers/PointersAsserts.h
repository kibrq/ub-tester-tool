#ifndef UB_POINTERS_ASSERTS_INCLUDE_H_
#define UB_POINTERS_ASSERTS_INCLUDE_H_

#include "ub-safe-containers/UBSafePointer.h"

namespace ub_tester {
#define ASSERT_STAROPERATOR(Pointer)                                           \
  ptr_checker::checkStarOperator(Pointer, __FILE__, __LINE__)

namespace ptr::checkers {

template <typename T>
T& checkStarOperator(UBSafePointer<T>& Pointer, const char* Filename,
                     size_t LineNum) {}

template <typename T>
const T& checkStarOperator(const UBSafePointer<T>& Pointer,
                           const char* Filename, size_t LineNum) {}

template <typename T>
T& checkStarOperator(T*& Pointer, const char* Filename, size_t LineNum) {}

template <typename T>
const T& checkStarOperator(const T*& Pointer, const char* Filename,
                           size_t LineNum) {}

} // namespace ptr::checkers
} // namespace ub_tester

#endif
