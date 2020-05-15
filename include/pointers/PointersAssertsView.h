#pragma once

#include <optional>
#include <sstream>
#include <vector>

namespace ub_tester::ptr::view {

namespace {
inline constexpr char AssertStarOperator[] = "ASSERT_STAROPERATOR";
inline constexpr char AssertMemberExpr[] = "ASSERT_MEMBEREXPR";
inline constexpr char SetSize[] = "setSize";
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

namespace {

inline void writeArgs(std::stringstream& Stream, const std::string& Arg) {
  Stream << Arg;
}

template <typename... Strings>
void writeArgs(std::stringstream& Stream, const std::string& Arg,
               const Strings&... Args) {
  Stream << Arg << ", ";
  writeArgs(Stream, Args...);
}

template <typename... Strings>
std::string generateAssert(const std::string& AssertName,
                           const Strings&... Args) {
  std::stringstream Stream;
  Stream << AssertName << "(";
  writeArgs(Stream, Args...);
  Stream << ")";
  return Stream.str();
}

} // namespace

inline std::string getAssertStarOpeartorAsString(const std::string& ArgName) {
  return generateAssert(AssertStarOperator, ArgName);
}

inline std::string getAssertMemberExprAsString(const std::string& PointerName,
                                               const std::string& MemberName) {
  return generateAssert(AssertMemberExpr, PointerName, MemberName);
}

} // namespace ub_tester::ptr::view
