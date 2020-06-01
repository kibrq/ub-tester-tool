#pragma once

#include "assert-message-manager/AssertMessageManager.h"
#include <string>

#define ASSERT_SET_VALUE(Variable, SetExpr) Variable.setValue(#SetExpr)
#define ASSERT_GET_VALUE(Variable) Variable.assertGetValue(__FILE__, __LINE__)
#define ASSERT_GET_REF(Variable) Variable.assertGetRef(__FILE__, __LINE__)
#define ASSERT_GET_REF_IGNORE(Variable) Variable.assertGetRefIgnore(__FILE__, __LINE__)

namespace ub_tester::uninit_vars {

using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;

inline std::string appendInfo(std::string msg, const char* Filename, int Line) {
  return msg + " in file \"" + Filename + "\" at line \"" + std::to_string(Line) + "\"";
}

template <typename T> class UBSafeType final {
public:
  UBSafeType() : value{}, isInit{false}, isIgnored{false} {}

  UBSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}

  UBSafeType(const UBSafeType<T>& t) : value{t.assertGetValue("unknown", -1)}, isInit{true}, isIgnored{false} {}

  T assertGetValue(const char* Filename, int Line) const {
    if (!isIgnored && !isInit) {
      PUSH_ERROR(UNINIT_VAR_ACCESS_ERROR, appendInfo("access to value of uninitialized variable", Filename, Line));
    }
    return value;
  }

  T& assertGetRef(const char* Filename, int Line) {
    if (!isIgnored && !isInit) {
      PUSH_ERROR(UNINIT_VAR_ACCESS_ERROR, appendInfo("access to value of uninitialized variable", Filename, Line));
    }
    return value;
  }

  T& assertGetRefIgnore(const char* Filename, int Line) {
    isIgnored = true;
    if (!isInit) {
      PUSH_WARNING(UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING, appendInfo("releasing an uninit variable", Filename, Line));
    }
    return value;
  }

  T& setValue(T t) {
    value = t;
    isInit = true;
    return value;
  }

  T& setValue(const UBSafeType<T>& t) {
    value = t.assertGetValue("unknown", -1);
    isInit = true;
    return value;
  }

  operator T() const {
    return assertGetValue("unknown", -1);
  }

  // the following operators DO NOT cause lvalue to rvalue cast, but need the value
  T& operator++() { return ++value; }

  T operator++(int) {
    T res = assertGetValue("unknown", -1);
    value++;
    return res;
  }

  T& operator--() { --value; }

  T operator--(int) {
    T res = assertGetValue("unknown", -1); 
    value--;
    return res;
  }

  T* operator&() { // ! not const
    if (!isInit) {
      isIgnored = true;
      PUSH_WARNING(UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING, appendInfo("releasing an uninit variable", "unknown", -1));
    }
    return &value;
  }

private:
  void check(const char* Filename, int Line) const {}

private:
  T value;
  bool isInit;
  bool isIgnored;
};

} // namespace ub_tester::uninit_vars