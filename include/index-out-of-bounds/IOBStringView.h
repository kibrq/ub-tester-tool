#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace ub_tester::iob::view {

namespace {

inline constexpr const char SizesTypename[] = "std::vector<int>";
inline constexpr const char IOBChecker[] = "ASSERT_INDEX_OUT_OF_BOUNDS";
inline constexpr const char InvalidSizeChecker[] = "ASSERT_INVALID_SIZE";

} // namespace

inline std::string
generateSafeArrayCtor(const std::vector<std::string>& Sizes,
                      const std::optional<std::string>& InitList) {
  std::stringstream SS;
  SS << InvalidSizeChecker << "(" << SizesTypename << "({";
  for (size_t i = 0; i < Sizes.size(); i++) {
    SS << Sizes[i];
    if (i != Sizes.size() - 1) {
      SS << ", ";
    }
  }
  SS << "}))";

  if (InitList.has_value()) {
    SS << ", " << InitList.value();
  }
  return SS.str();
}

inline std::string generateIOBChecker(const std::string& LHS,
                                      const std::string& RHS) {
  std::stringstream SS;
  SS << IOBChecker << "(" << LHS << ", " << RHS << ")";
  return SS.str();
}

} // namespace ub_tester::iob::view
