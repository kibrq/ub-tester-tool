#pragma once

#include <optional>
#include <sstream>

namespace ub_tester::ptr::view {

namespace {
inline constexpr const char AssertStarOperator[] = "ASSERT_STAROPERATOR";
inline constexpr const char SetSize[] = "setSize";
} // namespace

inline std::string
getSafePtrCtorAsString(const std::optional<std::string>& Initializer,
                       const std::optional<std::string>& Size) {
  std::stringstream Stream;
  Stream << "(" << Initializer.value_or("") << (Size.has_value() ? ", " : "")
         << (Size.value_or("")) << ")";
  return Stream.str();
}
inline std::string getSetSizeAsString(const std::string& PtrName,
                                      const std::optional<std::string>& Arg) {
  std::stringstream Stream;
  Stream << "(" << PtrName << ")."
         << "setSize(" << Arg.value_or("") << ")";
  return Stream.str();
}

inline std::string getAssertStarOpeartorAsString(const std::string& ArgName) {
  std::stringstream Stream;
  Stream << AssertStarOperator << "(" << ArgName << ")";
  return Stream.str();
}

} // namespace ub_tester::ptr::view
