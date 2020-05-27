#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace ub_tester::iob::names_to_inject {

namespace {

constexpr char SizesTypeName[] = "std::vector<int>";
constexpr char IOBAssertName[] = "ASSERT_IOB";
constexpr char InvalidSizeAssertName[] = "ASSERT_INVALID_SIZE";

} // namespace

inline std::string
generateSafeArrayCtor(const std::vector<std::string>& Sizes,
                      const std::optional<std::string>& InitList) {
  std::stringstream SStream;
  SStream << InvalidSizeAssertName << "(" << SizesTypeName << "({";
  for (size_t I = 0, Size = Sizes.size(); I < Size; ++I) {
    SStream << Sizes[I];
    if (I != Size - 1) {
      SStream << ", ";
    }
  }
  SStream << "}))";

  if (InitList.has_value()) {
    SStream << ", " << InitList.value();
  }
  return SStream.str();
}

inline std::string generateIOBAssertName(const std::string& Lhs,
                                         const std::string& Rhs) {
  std::stringstream SStream;
  SStream << IOBAssertName << "(" << Lhs << ", " << Rhs << ")";
  return SStream.str();
}

} // namespace ub_tester::iob::names_to_inject
