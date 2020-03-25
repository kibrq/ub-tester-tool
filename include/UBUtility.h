#pragma once

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"

namespace ub_tester {

struct var_info_ {
  std::string name_, size_, type_;
  bool should_visit_nodes_;
};

std::string
getExprAsString(const clang::Expr* ex, const clang::ASTContext* Context);

}; // namespace ub_tester
