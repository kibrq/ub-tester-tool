#include "index-out-of-bounds/IOBStringView.h"

#include <iomanip>
#include <sstream>

namespace {

constexpr const char* SizesTypename = "std::vector<int>";
constexpr const char* IOBChecker = "ASSERT_INDEX_OUT_OF_BOUNDS";
constexpr const char* InvalidSizeChecker = "ASSERT_INVALID_SIZE";

} // namespace

namespace ub_tester::iob_view {

std::string generateSafeArrayCtor(const std::vector<std::string>& Sizes,
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

std::string generateIOBChecker(const std::string& LHS, const std::string& RHS) {
  std::stringstream SS;
  SS << IOBChecker << "(" << LHS << ", " << RHS << ")";
  return SS.str();
}

} // namespace ub_tester::iob_view
