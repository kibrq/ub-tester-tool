#include <sstream>
#include <stdexcept>
#include <string>

namespace UB_UninitSafeTypeConsts {
const std::string TEMPLATE_NAME = "UB_UninitSafeType";
const std::string GETMETHOD_NAME = "getValue";
const std::string INITMETHOD_NAME = "setValue";
const std::string GETIGNOREMETHOD_NAME = "getRefIgnore";
} // namespace UB_UninitSafeTypeConsts

// this only needs to be included in target file; no other use
// TODO: require <string> or change to c-like string
template <typename T> class UB_UninitSafeType {
public:
  UB_UninitSafeType() : value{}, isInit{false}, isIgnored{false} {}
  UB_UninitSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}
  UB_UninitSafeType(const UB_UninitSafeType<T>& t) : value{t.getValue()}, isInit{true}, isIgnored{false} {}

  T getValue(std::string varName = std::string(), size_t line = 0) const {
    if (!isIgnored && !isInit) {
      std::stringstream errorMessage{"access to value of uninitialized variable"};
      if (varName != "")
        errorMessage << ' ' << varName;
      if (line != 0)
        errorMessage << " at " << line;
      throw std::logic_error(errorMessage.str());
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
    value = t.getValue();
    isInit = true;
    return value;
  }

private:
  T value;
  bool isInit;
  bool isIgnored;
};