#pragma once
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

#include <string>

namespace ub_tester {

std::string getExprAsString(const clang::Expr*, const clang::ASTContext*);

std::string getPointeeType(const clang::Type*);

}; // namespace ub_tester
