#pragma once

#include "ConfigInString.h"
#include <fstream>
#include <string>

namespace ub_tester::clio {

extern bool SuppressWarnings;
extern bool RunIOB;
extern bool RunArithm;
extern bool RunUninit;
extern bool SuppressAllOutput;

namespace internal {

enum ApplyOnly { IOB, Arithm, Uninit, All };
extern ApplyOnly AO;

} // namespace internal

inline void processFlags() {
  using namespace internal;
  switch (internal::AO) {
  case IOB: {
    RunIOB = true;
    RunArithm = false;
    RunUninit = false;
    break;
  }
  case Arithm: {
    RunIOB = false;
    RunArithm = true;
    RunUninit = false;
    break;
  }
  case Uninit: {
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
  using namespace internal::consts;
  std::ofstream ConfigOStream(ConfigName, std::ios::out);
  ConfigOStream << "#pragma once\n\n#define UBCONFIG_H_\n\n" << ConfigFlagsNamespace << " {\n";
  ConfigOStream << ConfigSuppressAllOutputFlagVariable << " = " << (SuppressAllOutput ? "true" : "false") << ";\n";
  ConfigOStream << ConfigSuppressWarningsFlagVariable << " = " << (SuppressWarnings ? "true" : "false") << ";\n";
  ConfigOStream << "} // " << ConfigFlagsNamespace;
  ConfigOStream.close();
}

} // namespace ub_tester::clio
