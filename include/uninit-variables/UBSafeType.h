#pragma once

#include "assert-message-manager/AssertMessageManager.h"
#include <string>

#define ASSERT_SET_VALUE(Variable, SetExpr) Variable.setValue_((SetExpr))
#define ASSERT_GET_VALUE(Variable) Variable.assertGetValue_(__FILE__, __LINE__)
#define ASSERT_GET_REF(Variable) Variable.assertGetRef(__FILE__, __LINE__)
#define ASSERT_GET_REF_IGNORE(Variable)                                        \
  Variable.assertGetRefIgnore(__FILE__, __LINE__)

namespace ub_tester::uninit_vars {

using assert_message_manager::AssertFailCode;
using assert_message_manager::AssertMessage;
using assert_message_manager::AssertMessageManager;

inline std::string appendInfo(std::string msg, const char* Filename, int Line) {
  return msg + " in \"" + Filename + "\" Line: \"" + std::to_string(Line) +
         "\"\n";
}

template <typename T>
class UBSafeType final {
public:
  UBSafeType() : Value_{}, IsInit_{false}, IsIgnored_{false} {}

  UBSafeType(T t) : Value_{t}, IsInit_{true}, IsIgnored_{false} {}

  UBSafeType(const UBSafeType<T>& t)
      : Value_{t.assertGetValue_("unknown", -1)}, IsInit_{true}, IsIgnored_{
                                                                     false} {}

  T assertGetValue_(const char* Filename, int Line) const {
    if (!IsIgnored_ && !IsInit_) {
      PUSH_ERROR(UNINIT_VAR_ACCESS_ERROR,
                 appendInfo("access to Value_ of uninitialized variable",
                            Filename, Line));
    }
    return Value_;
  }

  T& assertGetRef(const char* Filename, int Line) {
    if (!IsIgnored_ && !IsInit_) {
      PUSH_ERROR(UNINIT_VAR_ACCESS_ERROR,
                 appendInfo("access to Value_ of uninitialized variable",
                            Filename, Line));
    }
    return Value_;
  }

  T& assertGetRefIgnore(const char* Filename, int Line) {
    if (!IsIgnored_ && !IsInit_) {
      PUSH_WARNING(
          UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING,
          appendInfo("variable is not being tracked anymore", Filename, Line));
    }
    IsIgnored_ = true;
    return Value_;
  }

  T& setValue_(T Val) {
    Value_ = Val;
    IsInit_ = true;
    return Value_;
  }

  T& setValue_(const UBSafeType<T>& SafeTypeVar) {
    Value_ = SafeTypeVar.assertGetValue_("unknown", -1);
    IsInit_ = true;
    return Value_;
  }

  operator T() const { return assertGetValue_("unknown", -1); }

  // the following operators DO NOT cause lValue to rValue cast, but need the
  // Value
  T& operator++() { return ++Value_; }

  T operator++(int) {
    T res = assertGetValue_("unknown", -1);
    Value_++;
    return res;
  }

  T& operator--() { --Value_; }

  T operator--(int) {
    T res = assertGetValue_("unknown", -1);
    Value_--;
    return res;
  }

  T* operator&() { // ! not const
    if (!IsInit_) {
      IsIgnored_ = true;
      PUSH_WARNING(UNINIT_VAR_IS_NOT_TRACKED_ANYMORE_WARNING,
                   appendInfo("releasing an uninit variable", "unknown", -1));
    }
    return &Value_;
  }

private:
  T Value_;
  bool IsInit_;
  bool IsIgnored_;
};

} // namespace ub_tester::uninit_vars
