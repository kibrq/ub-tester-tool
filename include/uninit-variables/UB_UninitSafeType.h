#include "../assert-message-manager/AssertMessageManager.h"

#include <stdexcept>
#include <string>

// this only needs to be included in target file; no other use
// TODO: require <string> or change to c-like string
template <typename T>
class UB_UninitSafeType {
public:
  UB_UninitSafeType() : value{}, isInit{false}, isIgnored{false} {}
  UB_UninitSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}
  UB_UninitSafeType(const UB_UninitSafeType<T>& t)
      : value{t.getValue()}, isInit{true}, isIgnored{false} {}

  struct CallInfo {
    const std::string file = "";
    size_t line = 0;
    const std::string varName = "";
    const std::string varType = "";
  };

  T getValue(CallInfo varInfo) const {
    check(varInfo);
    return value;
  }
  T& getRefCheck(CallInfo varInfo) {
    check(varInfo);
    return value;
  }
  T& getRefIgnore(CallInfo varInfo) {
    isIgnored = true;
    if (!isInit) {
      // TODO: warning if not init yet
      using namespace ub_tester;
      using assert_message_manager::AssertFailCode;
      using assert_message_manager::AssertMessage;
      using assert_message_manager::AssertMessageManager;
      std::string warningMessage = "releasing an uninit variable";
      appendInfo(warningMessage, varInfo);
      PUSH_WARNING(UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING, warningMessage);
    }
    return value;
  }
  T& setValue(T t) {
    value = t;
    isInit = true;
    return value;
  }
  T& setValue(const UB_UninitSafeType<T>& t) {
    value = t.getValue({}); // TODO: avoid line mismatch
    // ? need to pass CallInfo, but how to in T() or operator&() ?
    isInit = true;
    return value;
  }
  operator T() const {
    // TODO: receive CallInfo
    return getValue({}); // TODO: avoid line mismatch
  }
  // the following unary operators DO NOT cause lvalue to rvalue cast
  // yet they DO cause UB with uninit vars
  T& operator++() { return ++value; }
  T operator++(int) {
    // ? TODO: send a disclaimer?
    T res = getValue({}); // TODO: line mismatch
    value++;
    return res;
  }
  T& operator--() { --value; }
  T operator--(int) {
    // ? TODO: disclaimer?
    T res = getValue({}); // TODO: line mismatch
    value--;
    return res;
  }
  T* operator&() { // ! not const
    if (!isInit) {
      isIgnored = true;
      // TODO: send warning
    }
    return &value;
  }
  // also all compound operators
  // ! redundant since now explicitly checked
  // template <typename U = T> auto& operator+=(U u) { return value += u; }
  // template <typename U = T> auto& operator-=(U u) { return value -= u; }
  // template <typename U = T> auto& operator*=(U u) { return value *= u; }
  // template <typename U = T> auto& operator/=(U u) { return value /= u; }
  // template <typename U = T> auto& operator%=(U u) { return value %= u; }
  // template <typename U = T> auto& operator&=(U u) { return value &= u; }
  // template <typename U = T> auto& operator|=(U u) { return value |= u; }
  // template <typename U = T> auto& operator^=(U u) { return value ^= u; }
  // template <typename U = T> auto& operator<<=(U u) { return value ^= u; }
  // template <typename U = T> auto& operator>>=(U u) { return value ^= u; }

private:
  T value;
  bool isInit;
  bool isIgnored;

  static void appendInfo(std::string& msg, CallInfo varInfo) {
    if (varInfo.varName != "")
      msg += (" named \'" + varInfo.varName + "\'");
    if (varInfo.varType != "")
      msg += (" of type \'" + varInfo.varType + "\'");
    if (varInfo.file != "")
      msg += (" in file " + varInfo.file);
    if (varInfo.line)
      msg += (" at line " + std::to_string(varInfo.line));
  }

  void check(CallInfo varInfo) const {
    using namespace ub_tester;
    using assert_message_manager::AssertFailCode;
    using assert_message_manager::AssertMessage;
    using assert_message_manager::AssertMessageManager;
    if (!isIgnored && !isInit) {
      std::string errorMessage{"access to value of uninitialized variable"};
      appendInfo(errorMessage, varInfo);
      PUSH_WARNING(UNINIT_VAR_ACCESS_ERROR, errorMessage);
    }
  }
};
