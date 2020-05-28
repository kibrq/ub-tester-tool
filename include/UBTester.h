#ifndef UBTESTER_INCLUDE_H_
#define UBTESTER_INCLUDE_H_

#include "index-out-of-bounds/IOBAsserts.h"
#include "ub-safe-containers/UBSafeCArray.h"
#include "ub-safe-containers/UBSafePointer.h"

#include "pointers/PointersAsserts.h"

#include "arithmetic-overflow/ArithmeticUBAsserts.h"

#include "uninit-variables/UB_UninitSafeType.h"

using ub_tester::UBSafeCArray;
using ub_tester::UBSafePointer;
using ub_tester::uninit_vars::UBSafeType;

#endif
