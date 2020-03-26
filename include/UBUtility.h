#pragma once

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"

using namespace clang;

namespace ub_tester {

std::string getExprAsString(const clang::Expr* ex, const ASTContext* Context);
std::string getExprLineNCol(const Expr* Expression, const ASTContext* Context);

}; // namespace ub_tester
