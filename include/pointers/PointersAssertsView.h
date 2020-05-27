#pragma once

#include <optional>
#include <sstream>
#include <vector>

namespace ub_tester::ptr::names_to_inject {

namespace {
constexpr char AssertStarOperatorName[] = "ASSERT_STAROPERATOR";
constexpr char AssertMemberExprName[] = "ASSERT_MEMBEREXPR";
constexpr char SetSizeMethodName[] = "setSize";
} // namespace

inline std::string
getSafePtrCtorAsString(const std::optional<std::string>& Initializer,
                       const std::optional<std::string>& Size) {
  std::stringstream SStream;
  SStream << "(" << Initializer.value_or("") << (Size.has_value() ? ", " : "")
          << (Size.value_or("")) << ")";
  return SStream.str();
}
inline std::string getSetSizeAsString(const std::string& PtrName,
                                      const std::optional<std::string>& Arg) {
  std::stringstream SStream;
  SStream << "(" << PtrName << ")."
          << "setSize(" << Arg.value_or("") << ")";
  return SStream.str();
}

namespace {

inline void writeArgs(std::stringstream& SStream, const std::string& Arg) {
  SStream << Arg;
}

template <typename... Strings>
void writeArgs(std::stringstream& SStream, const std::string& Arg,
               const Strings&... Args) {
  SStream << Arg << ", ";
  writeArgs(SStream, Args...);
}

template <typename... Strings>
std::string generateAssertHelper(const std::string& AssertName,
                                 const Strings&... Args) {
  std::stringstream SStream;
  SStream << AssertName << "(";
  writeArgs(SStream, Args...);
  SStream << ")";
  return SStream.str();
}

} // namespace

inline std::string getAssertStarOperatorAsString(const std::string& ArgName) {
  return generateAssertHelper(AssertStarOperatorName, ArgName);
}

inline std::string getAssertMemberExprAsString(const std::string& PointerName,
                                               const std::string& MemberName) {
  return generateAssertHelper(AssertMemberExprName, PointerName, MemberName);
}

} // namespace ub_tester::ptr::names_to_inject
