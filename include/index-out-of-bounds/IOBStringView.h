#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ub_tester {

std::string
generateSafeArrayTypename(size_t Dimension, const std::string& Type);
std::string generateSafeArrayCtor(
    const std::vector<std::string>& Sizes,
    const std::optional<std::string> InitList);

std::string generateIOBChecker(const std::string& LHS, const std::string& RHS);

} // namespace ub_tester
