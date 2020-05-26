#pragma once

// extern int DInt;

namespace ub_tester::clio {

extern bool SuppressWarnings;
extern bool runOOB;
extern bool runArithm;
extern bool runUninit;

namespace internal {
enum ApplyOnly { OOB, Arithm, Uninit, All };
extern ApplyOnly AO;
} // namespace internal

inline void processFlags() {
  using namespace internal;
  switch (internal::AO) {
  case OOB: {
    runOOB = true;
    runArithm = false;
    runUninit = false;
    break;
  }
  case Arithm: {
    runOOB = false;
    runArithm = true;
    runUninit = false;
    break;
  }
  case Uninit: {
    runOOB = false;
    runArithm = false;
    runUninit = true;
    break;
  }
  default: {
    runOOB = true;
    runArithm = true;
    runUninit = true;
  }
  }
}

} // namespace ub_tester::clio
