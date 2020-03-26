#pragma once

#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"

namespace ub_tester {

std::string getExprAsString(const clang::Expr*, const clang::ASTContext*);
std::string getExprLineNCol(const clang::Expr*, const clang::ASTContext*);

}; // namespace ub_tester
