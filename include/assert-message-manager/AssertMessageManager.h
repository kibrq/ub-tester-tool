#pragma once

#ifdef UB_TESTER
  #include ".UBConfig.h"
#endif
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define PUSH_ERROR(FailCode, Message)                                                                                            \
  AssertMessageManager::pushMessage(AssertMessage((Message), AssertFailCode::FailCode));                                         \
  assert(0 && "Assert detected error but manager didn't handle it")
#define PUSH_WARNING(FailCode, Message)                                                                                          \
  AssertMessageManager::pushMessage(AssertMessage("warning! " + (Message), AssertFailCode::FailCode))

namespace ub_tester::assert_message_manager::supress_messages_mode {

#ifndef UB_TESTER
  constexpr bool SUPRESS_ALL = false;
  constexpr bool SUPRESS_WARNINGS = false;
#endif

constexpr bool SUPRESS_ARITHM_WARNINGS = false;
constexpr bool SUPRESS_UNSIGNED_OVERFLOW_WARNING = false;
constexpr bool SUPRESS_OVERFLOW_IN_BITSHIFT_CXX20_WARNING = false;
constexpr bool SUPRESS_UNSAFE_CONV_WARNING = false;
constexpr bool SUPRESS_IMPL_DEFINED_UNSAFE_CONV_WARNING = false;

constexpr bool SUPRESS_UNINIT_VARS_WARNINGS = false;
// constexpr bool SUPRESS_IOB_WARNINGS = false;

constexpr bool SUPRESS_IMPL_DEFINED_WARNING = false;
constexpr bool SUPRESS_NOT_CONSIDERED_WARNING = false;

}; // namespace ub_tester::assert_message_manager::supress_messages_mode

namespace ub_tester::assert_message_manager {

enum class AssertFailCode { // error code > 0, warning code < 0
  OVERFLOW_ERROR = 1,
  DIVISION_BY_ZERO_ERROR = 2,
  UNDEFINED_MOD_ERROR = 3,
  UNDEFINED_BITSHIFT_LEFT_ERROR = 4,
  UNDEFINED_BITSHIFT_RIGHT_ERROR = 5,
  UNINIT_VAR_ACCESS_ERROR = 20,

  UNSIGNED_OVERFLOW_WARNING = -1,
  OVERFLOW_IN_BITSHIFT_CXX20_WARNING = -2,
  IMPL_DEFINED_WARNING = -3,
  UNSAFE_CONV_WARNING = -4,
  IMPL_DEFINED_UNSAFE_CONV_WARNING = -5,
  NOT_CONSIDERED_WARNING = -6,
  UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING = -20
};

struct AssertMessage final {
  AssertMessage(std::string Message, AssertFailCode FailCode) : Message_{Message}, FailCode_{FailCode} {}
  std::string Message_;
  AssertFailCode FailCode_;
};

bool checkIfMessageIsSupressed(AssertFailCode FailCode) {
  using namespace supress_messages_mode;
  if (SUPRESS_ALL)
    return true;
  if (SUPRESS_WARNINGS && static_cast<int>(FailCode) < 0)
    return true;

  switch (FailCode) {
  // arithm errors
  case AssertFailCode::OVERFLOW_ERROR:
    return false;
  case AssertFailCode::DIVISION_BY_ZERO_ERROR:
    return false;
  case AssertFailCode::UNDEFINED_MOD_ERROR:
    return false;
  case AssertFailCode::UNDEFINED_BITSHIFT_LEFT_ERROR:
    return false;
  case AssertFailCode::UNDEFINED_BITSHIFT_RIGHT_ERROR:
    return false;
  // uninit-vars error
  case AssertFailCode::UNINIT_VAR_ACCESS_ERROR:
    return false;
  // arithm warnings
  case AssertFailCode::UNSIGNED_OVERFLOW_WARNING:
    return SUPRESS_UNSIGNED_OVERFLOW_WARNING || SUPRESS_ARITHM_WARNINGS;
  case AssertFailCode::OVERFLOW_IN_BITSHIFT_CXX20_WARNING:
    return SUPRESS_OVERFLOW_IN_BITSHIFT_CXX20_WARNING || SUPRESS_ARITHM_WARNINGS;
  case AssertFailCode::IMPL_DEFINED_WARNING:
    return SUPRESS_IMPL_DEFINED_WARNING || SUPRESS_ARITHM_WARNINGS;
  case AssertFailCode::UNSAFE_CONV_WARNING:
    return SUPRESS_UNSAFE_CONV_WARNING || SUPRESS_ARITHM_WARNINGS;
  case AssertFailCode::IMPL_DEFINED_UNSAFE_CONV_WARNING:
    return SUPRESS_IMPL_DEFINED_UNSAFE_CONV_WARNING || SUPRESS_ARITHM_WARNINGS;
  case AssertFailCode::NOT_CONSIDERED_WARNING:
    return SUPRESS_NOT_CONSIDERED_WARNING || SUPRESS_ARITHM_WARNINGS;
  // uninit-vars warning
  case AssertFailCode::UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING:
    return SUPRESS_UNINIT_VARS_WARNINGS;
  }
  assert(0 && "Undefined AssertFailCode");
}

class AssertMessageManager final {
private:
  AssertMessageManager() = default;

  void handleMessage(AssertMessage Message) {
    if (!checkIfMessageIsSupressed(Message.FailCode_))
      Messages_.push_back(std::move(Message));
    if (static_cast<int>(Message.FailCode_) > 0) // if error
      printMessagesNTerminate(Message.FailCode_);
  }

  void printMessagesNTerminate(AssertFailCode FailCode) {
    for (const auto& Message : Messages_)
      std::cerr << Message.Message_ << "\n";
    Messages_.clear();
    if (!supress_messages_mode::SUPRESS_ALL)
      std::cerr << "error detected, aborting\n\n";
    exit(static_cast<int>(FailCode));
  }

public:
  ~AssertMessageManager() {
    for (const auto& Message : Messages_)
      std::cerr << Message.Message_ << "\n";
  }

  static AssertMessageManager& getInstance() {
    if (!ManagerPtr_)
      ManagerPtr_ = std::unique_ptr<AssertMessageManager>(new AssertMessageManager{});
    return *ManagerPtr_;
  }

  static void pushMessage(AssertMessage Message) { AssertMessageManager::getInstance().handleMessage(std::move(Message)); }

private:
  static std::unique_ptr<AssertMessageManager> ManagerPtr_;
  std::vector<AssertMessage> Messages_{};
};

inline std::unique_ptr<AssertMessageManager> AssertMessageManager::ManagerPtr_ = nullptr;

} // namespace ub_tester::assert_message_manager
