#pragma once

#include "ConfigInString.h"
#include <fstream>
#include <string>

namespace ub_tester::cli {

extern bool SuppressWarnings;
extern bool RunIOB;
extern bool RunArithm;
extern bool RunUninit;
extern bool SuppressAllOutput;

namespace internal {

enum ApplyOnly { IOB, Arithm, Uninit, All };
extern ApplyOnly CheckToApply;

} // namespace internal

inline void processFlags() {
  using namespace internal::consts;
  using namespace internal;
  switch (internal::CheckToApply) {
  case ApplyOnly::IOB: {
    RunIOB = true;
    RunArithm = false;
    RunUninit = false;
    break;
  }
  case ApplyOnly::Arithm: {
    RunIOB = false;
    RunArithm = true;
    RunUninit = false;
    break;
  }
  case ApplyOnly::Uninit: {
    RunIOB = false;
    RunArithm = false;
    RunUninit = true;
    break;
  }
  default: {
    RunIOB = true;
    RunArithm = true;
    RunUninit = true;
  }
  }
  // generate manager header
  std::ofstream ConfigOStream(ConfigName, std::ios::out);
  ConfigOStream << "#pragma once\n\n#define UBCONFIG_H_\n\n"
                << ConfigFlagsNamespace << " {\n";
  ConfigOStream << ConfigSuppressAllOutputFlagVariableName << " = "
                << (SuppressAllOutput ? "true" : "false") << ";\n";
  ConfigOStream << ConfigSuppressWarningsFlagVariableName << " = "
                << (SuppressWarnings ? "true" : "false") << ";\n";
  ConfigOStream << "} // " << ConfigFlagsNamespace;
  ConfigOStream.close();
}

} // namespace ub_tester::cli
