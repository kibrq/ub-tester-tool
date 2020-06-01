#ifndef UBTESTER_INCLUDE_H_
#define UBTESTER_INCLUDE_H_

#include "ub-safe-containers/IOBAsserts.h"
#include "ub-safe-containers/UBSafeCArray.h"
#include "ub-safe-containers/UBSafePointer.h"

#include "ub-safe-containers/PointerUBAsserts.h"

#include "arithmetic-ub/ArithmeticUBAsserts.h"

#include "uninit-variables/UB_UninitSafeType.h"

using ub_tester::UBSafeCArray;
using ub_tester::UBSafePointer;
using ub_tester::uninit_vars::UBSafeType;

#endif
