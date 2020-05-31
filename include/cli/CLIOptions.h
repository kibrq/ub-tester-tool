#pragma once

#include "AssertMessageManagerInString.h"
#include <fstream>
#include <string>

namespace ub_tester::clio {

extern bool SuppressWarnings;
extern bool RunOOB;
extern bool RunArithm;
extern bool RunUninit;
extern bool SuppressAllOutput;

namespace internal {

enum ApplyOnly { OOB, Arithm, Uninit, All };
extern ApplyOnly AO;

} // namespace internal

inline void processFlags() {
  using namespace internal;
  switch (internal::AO) {
  case OOB: {
    RunOOB = true;
    RunArithm = false;
    RunUninit = false;
    break;
  }
  case Arithm: {
    RunOOB = false;
    RunArithm = true;
    RunUninit = false;
    break;
  }
  case Uninit: {
    RunOOB = false;
    RunArithm = false;
    RunUninit = true;
    break;
  }
  default: {
    RunOOB = true;
    RunArithm = true;
    RunUninit = true;
  }
  }
  // generate manager header
  using namespace internal::consts;
  std::ofstream ManagerOStream(ManagerName, std::ios::out);
  ManagerOStream << ManagerInStringBeforeFlags << ManagerSuppressAllOutputFlagVariable << " = "
                 << (SuppressAllOutput ? "true" : "false") << ";\n"
                 << ManagerSuppressWarningsFlagVariable << " = " << (SuppressWarnings ? "true" : "false") << ";\n"
                 << ManagerInStringAfterArgs;
  ManagerOStream.close();
}

} // namespace ub_tester::clio
