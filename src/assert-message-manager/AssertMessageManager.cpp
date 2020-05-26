#include "assert-message-manager/AssertMessageManager.h"
#include <cassert>
#include <iostream>

namespace ub_tester::assert_message_manager {

std::unique_ptr<AssertMessageManager> AssertMessageManager::ManagerPtr_ =
    nullptr;

AssertMessage::AssertMessage(std::string Message,
                             AssertFailCode FailCode) noexcept
    : Message_{Message}, FailCode_{FailCode} {}

AssertMessageManager::~AssertMessageManager() noexcept {
  for (const auto& Message : Messages_)
    std::cerr << Message.Message_;
}

AssertMessageManager& AssertMessageManager::getInstance() noexcept {
  if (!ManagerPtr_)
    ManagerPtr_ =
        std::unique_ptr<AssertMessageManager>(new AssertMessageManager{});
  return *ManagerPtr_;
}

void AssertMessageManager::handleMessage(AssertMessage Message) noexcept {
  if (static_cast<int>(Message.FailCode_) > 0) // if error, not a warning
    printMessagesNTerminate(Message.FailCode_);
  Messages_.push_back(std::move(Message));
}

void AssertMessageManager::printMessagesNTerminate(
    AssertFailCode FailCode) noexcept {
  for (const auto& Message : Messages_)
    std::cerr << Message.Message_;
  exit(static_cast<int>(FailCode));
}

void AssertMessageManager::pushMessage(AssertMessage Message) noexcept {
  AssertMessageManager::getInstance().handleMessage(std::move(Message));
}

} // namespace ub_tester::assert_message_manager
