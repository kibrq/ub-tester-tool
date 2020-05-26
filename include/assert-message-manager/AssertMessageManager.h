#pragma once

#include <cassert>
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

struct AssertMessage {
  AssertMessage(std::string Message, AssertFailCode FailCode) noexcept;
  std::string Message_;
  AssertFailCode FailCode_;
};

class AssertMessageManager final {
private:
  AssertMessageManager() = default;
  void handleMessage(AssertMessage Message) noexcept;
  void printMessagesNTerminate(AssertFailCode FailCode) noexcept;

public:
  ~AssertMessageManager() noexcept;
  static AssertMessageManager& getInstance() noexcept;
  static void pushMessage(AssertMessage Message) noexcept;

private:
  static std::unique_ptr<AssertMessageManager> ManagerPtr_;
  std::vector<AssertMessage> Messages_{};
};

} // namespace ub_tester::assert_message_manager
