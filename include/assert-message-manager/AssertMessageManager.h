#pragma once

#include <cassert>
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

namespace ub_tester::assert_message_manager::suppress_messages_mode {

#ifndef UB_TESTER
constexpr bool SUPPRESS_ALL = false;
constexpr bool SUPPRESS_WARNINGS = false;
#endif

constexpr bool SUPPRESS_ARITHM_WARNINGS = false;
constexpr bool SUPPRESS_UNSIGNED_OVERFLOW_WARNING = false;
constexpr bool SUPPRESS_OVERFLOW_IN_BITSHIFT_CXX20_WARNING = false;
constexpr bool SUPPRESS_UNSAFE_CONV_WARNING = false;
constexpr bool SUPPRESS_IMPL_DEFINED_UNSAFE_CONV_WARNING = false;

constexpr bool SUPPRESS_UNINIT_VARS_WARNINGS = false;
constexpr bool SUPPRESS_PTR_WARNINGS = false;

constexpr bool SUPPRESS_IMPL_DEFINED_WARNING = false;
constexpr bool SUPPRESS_NOT_CONSIDERED_WARNING = false;

}; // namespace ub_tester::assert_message_manager::suppress_messages_mode

namespace ub_tester::assert_message_manager {

enum class AssertFailCode { // error code > 0, warning code < 0
  OVERFLOW_ERROR = 1,
  DIVISION_BY_ZERO_ERROR = 2,
  UNDEFINED_MOD_ERROR = 3,
  UNDEFINED_BITSHIFT_LEFT_ERROR = 4,
  UNDEFINED_BITSHIFT_RIGHT_ERROR = 5,
  UNINIT_VAR_ACCESS_ERROR = 20,
  INDEX_OUT_OF_BOUNDS_ERROR = 10,
  INVALID_SIZE_OF_ARRAY_ERROR = 11,
  NULLPTR_DEREF_ERROR = 12,
  UNINIT_PTR_DEREF_ERROR = 13,

  UNSIGNED_OVERFLOW_WARNING = -1,
  OVERFLOW_IN_BITSHIFT_CXX20_WARNING = -2,
  IMPL_DEFINED_WARNING = -3,
  UNSAFE_CONV_WARNING = -4,
  IMPL_DEFINED_UNSAFE_CONV_WARNING = -5,
  NOT_CONSIDERED_WARNING = -6,
  UNTRACKED_PTR_WARNING = -10,
  UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING = -20
};

struct AssertMessage final {
  AssertMessage(std::string Message, AssertFailCode FailCode) : Message_{Message}, FailCode_{FailCode} {}
  std::string Message_;
  AssertFailCode FailCode_;
};

bool checkIfMessageIsSuppressed(AssertFailCode FailCode) {
  using namespace suppress_messages_mode;
  if (SUPPRESS_ALL)
    return true;
  if (SUPPRESS_WARNINGS && static_cast<int>(FailCode) < 0)
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
  // iob error
  case AssertFailCode::INDEX_OUT_OF_BOUNDS_ERROR:
    return false;
  case AssertFailCode::INVALID_SIZE_OF_ARRAY_ERROR:
    return false;
  case AssertFailCode::NULLPTR_DEREF_ERROR:
    return false;
  case AssertFailCode::UNINIT_PTR_DEREF_ERROR:
    return false;
  // arithm warnings
  case AssertFailCode::UNSIGNED_OVERFLOW_WARNING:
    return SUPPRESS_UNSIGNED_OVERFLOW_WARNING || SUPPRESS_ARITHM_WARNINGS;
  case AssertFailCode::OVERFLOW_IN_BITSHIFT_CXX20_WARNING:
    return SUPPRESS_OVERFLOW_IN_BITSHIFT_CXX20_WARNING || SUPPRESS_ARITHM_WARNINGS;
  case AssertFailCode::IMPL_DEFINED_WARNING:
    return SUPPRESS_IMPL_DEFINED_WARNING || SUPPRESS_ARITHM_WARNINGS;
  case AssertFailCode::UNSAFE_CONV_WARNING:
    return SUPPRESS_UNSAFE_CONV_WARNING || SUPPRESS_ARITHM_WARNINGS;
  case AssertFailCode::IMPL_DEFINED_UNSAFE_CONV_WARNING:
    return SUPPRESS_IMPL_DEFINED_UNSAFE_CONV_WARNING || SUPPRESS_ARITHM_WARNINGS;
  case AssertFailCode::NOT_CONSIDERED_WARNING:
    return SUPPRESS_NOT_CONSIDERED_WARNING || SUPPRESS_ARITHM_WARNINGS;
  // uninit-vars warning
  case AssertFailCode::UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING:
    return SUPPRESS_UNINIT_VARS_WARNINGS;
  // iob warnings
  case AssertFailCode::UNTRACKED_PTR_WARNING:
    return SUPPRESS_PTR_WARNINGS;
  }
  assert(0 && "Undefined AssertFailCode");
}

class AssertMessageManager final {
private:
  AssertMessageManager() = default;

  void handleMessage(AssertMessage Message) {
    if (!checkIfMessageIsSuppressed(Message.FailCode_))
      Messages_.push_back(std::move(Message));
    if (static_cast<int>(Message.FailCode_) > 0) // if error
      printMessagesNTerminate(Message.FailCode_);
  }

  void printMessagesNTerminate(AssertFailCode FailCode) {
    for (const auto& Message : Messages_)
      std::cerr << Message.Message_ << "\n";
    Messages_.clear();
    if (!suppress_messages_mode::SUPPRESS_ALL)
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
