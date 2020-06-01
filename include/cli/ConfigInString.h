#pragma once

#include <string>

namespace ub_tester::clio::internal::consts {

inline const std::string ConfigName = ".UBConfig.h";
inline const std::string ConfigFlagsNamespace = "namespace ub_tester::assert_message_manager::suppress_messages_mode";
inline const std::string ConfigSuppressAllOutputFlagVariable = "constexpr bool SUPPRESS_ALL";     // = false;
inline const std::string ConfigSuppressWarningsFlagVariable = "constexpr bool SUPPRESS_WARNINGS"; // = false;

} // namespace ub_tester::clio::internal::consts