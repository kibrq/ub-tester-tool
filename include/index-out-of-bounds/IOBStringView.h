#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ub_tester {
namespace iob_view {

std::string generateSafeArrayTypename(bool isStatic, size_t Dimension, const std::string& Type);
std::string generateSafeArrayCtor(const std::vector<std::string>& Sizes,
                                  const std::optional<std::string>& InitList);

std::string generateIOBChecker(const std::string& LHS, const std::string& RHS);

} // namespace iob_view
} // namespace ub_tester
