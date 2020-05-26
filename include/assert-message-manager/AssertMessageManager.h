#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ub_tester::assert_message_manager {

enum class AssertFailCode { // error code > 0, warning code < 0
  OVERFLOW_ERROR = 1,
  DIVISION_BY_ZERO_ERROR = 2,
  UNDEFINED_MOD_ERROR = 3,
  UNDEFINED_BITSHIFT_LEFT_ERROR = 4,
  UNDEFINED_BITSHIFT_RIGHT_ERROR = 5,

  UNSIGNED_OVERFLOW_WARNING = -1,
  OVERFLOW_IN_BITSHIFT_CXX20_WARNING = -2,
  IMPL_DEFINED_WARNING = -3,
  UNSAFE_CONV_WARNING = -4,
  IMPL_DEFINED_UNSAFE_CONV_WARNING = -5,
  NOT_CONSIDERED_WARNING = -6
};

struct AssertMessage final {
  AssertMessage(std::string Message, AssertFailCode FailCode) noexcept
      : Message_{Message}, FailCode_{FailCode} {}
  std::string Message_;
  AssertFailCode FailCode_;
};

class AssertMessageManager final {
private:
  AssertMessageManager() = default;

  void handleMessage(AssertMessage Message) noexcept {
    Messages_.push_back(std::move(Message));
    if (static_cast<int>(Message.FailCode_) > 0) // if error, not a warning
      printMessagesNTerminate(Message.FailCode_);
  }

  void printMessagesNTerminate(AssertFailCode FailCode) noexcept {
    for (const auto& Message : Messages_)
      std::cerr << Message.Message_ << "\n";
    Messages_.clear();
    exit(static_cast<int>(FailCode));
  }

public:
  ~AssertMessageManager() noexcept {
    for (const auto& Message : Messages_)
      std::cerr << Message.Message_ << "\n";
  }

  static AssertMessageManager& getInstance() noexcept {
    if (!ManagerPtr_)
      ManagerPtr_ =
          std::unique_ptr<AssertMessageManager>(new AssertMessageManager{});
    return *ManagerPtr_;
  }

  static void pushMessage(AssertMessage Message) noexcept {
    AssertMessageManager::getInstance().handleMessage(std::move(Message));
  }

private:
  static std::unique_ptr<AssertMessageManager> ManagerPtr_;
  std::vector<AssertMessage> Messages_{};
};

inline std::unique_ptr<AssertMessageManager> AssertMessageManager::ManagerPtr_ =
    nullptr;

} // namespace ub_tester::assert_message_manager
