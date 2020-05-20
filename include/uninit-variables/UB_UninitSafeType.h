#include <stdexcept>
#include <string>

// this only needs to be included in target file; no other use
// TODO: require <string> or change to c-like string
template <typename T> class UB_UninitSafeType {
public:
  UB_UninitSafeType() : value{}, isInit{false}, isIgnored{false} {}
  UB_UninitSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}
  UB_UninitSafeType(const UB_UninitSafeType<T>& t) : value{t.getValue()}, isInit{true}, isIgnored{false} {}

  struct CallInfo {
    const std::string file = "";
    size_t line = 0;
    const std::string varName = "";
    const std::string varType = "";
  };

  T getValue(CallInfo varInfo) const {
    if (!isIgnored && !isInit) {
      std::string errorMessage{"access to value of uninitialized variable"};
      if (varInfo.varName != "")
        errorMessage += (" named \'" + varInfo.varName + "\'");
      if (varInfo.varType != "")
        errorMessage += (" of type \'" + varInfo.varType + "\'");
      if (varInfo.file != "")
        errorMessage += (" in file " + varInfo.file);
      if (varInfo.line)
        errorMessage += (" at line " + std::to_string(varInfo.line));
      throw std::logic_error(errorMessage);
    }
    return value;
  }
  // T& getReference() { return value; }
  T& getRefIgnore() {
    isIgnored = true;
    return value;
    // TODO: general warning
    // TODO: extra warning if not init yet (?)
  }
  T& setValue(T t) {
    value = t;
    isInit = true;
    return value;
  }
  T& setValue(const UB_UninitSafeType<T>& t) {
    value = t.getValue({}); // TODO: avoid line mismatch
    isInit = true;
    return value;
  }
  operator T() const {
    // TODO: receive CallInfo
    return getValue({}); // TODO: avoid line mismatch
  }
  // the following unary operators DO NOT cause lvalue to rvalue cast
  T& operator++() { return ++value; }
  T operator++(int) {
    // ? TODO: send a disclaimer?
    // we used the value; however, it is somehow unused irl ahaha in fact it is
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
  template <typename U = T> auto& operator+=(U u) { return value += u; }
  template <typename U = T> auto& operator-=(U u) { return value -= u; }
  template <typename U = T> auto& operator*=(U u) { return value *= u; }
  template <typename U = T> auto& operator/=(U u) { return value /= u; }
  template <typename U = T> auto& operator%=(U u) { return value %= u; }
  template <typename U = T> auto& operator&=(U u) { return value &= u; }
  template <typename U = T> auto& operator|=(U u) { return value |= u; }
  template <typename U = T> auto& operator^=(U u) { return value ^= u; }
  template <typename U = T> auto& operator<<=(U u) { return value ^= u; }
  template <typename U = T> auto& operator>>=(U u) { return value ^= u; }

private:
  T value;
  bool isInit;
  bool isIgnored;
};