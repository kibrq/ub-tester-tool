#pragma once

#include <string>

namespace ub_tester::cli::internal::consts {

inline const std::string ConfigName = ".UBConfig.h";
inline const std::string ConfigFlagsNamespace =
    "namespace ub_tester::assert_message_manager::suppress_messages_mode";
inline const std::string ConfigSuppressAllOutputFlagVariableName =
    "constexpr bool SUPPRESS_ALL"; // = false;
inline const std::string ConfigSuppressWarningsFlagVariableName =
    "constexpr bool SUPPRESS_WARNINGS"; // = false;

} // namespace ub_tester::cli::internal::consts
