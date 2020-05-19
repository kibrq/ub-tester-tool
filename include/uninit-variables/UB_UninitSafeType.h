#include <stdexcept>

// this only needs to be included in target file; no other use
// TODO: require <string> or change to c-like string
template <typename T> class UB_UninitSafeType {
public:
  UB_UninitSafeType() : value{}, isInit{false}, isIgnored{false} {}
  UB_UninitSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}
  UB_UninitSafeType(const UB_UninitSafeType<T>& t) : value{t.getValue()}, isInit{true}, isIgnored{false} {}

  struct VariableInfo {
    const char* filename = nullptr;
    size_t line = 0;
    const char* varName = nullptr;
    const char* varType = nullptr;
  };

  T getValue(VariableInfo varInfo) const {
    if (!isIgnored && !isInit) {
      std::string errorMessage{"access to value of uninitialized variable"};
      if (varInfo.varName)
        errorMessage += " " + varInfo.varName;
      if (varInfo.varType)
        errorMessage += " of type \'" + varInfo.varType + "\'";
      if (varInfo.file)
        errorMessage += " in file " + varInfo.file;
      if (varInfo.line)
        errorMessage += " at line " + varInfo.line;
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
    value = t.getValue(); // TODO: avoid line mismatch
    isInit = true;
    return value;
  }

private:
  T value;
  bool isInit;
  bool isIgnored;
};