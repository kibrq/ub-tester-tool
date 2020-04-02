#pragma once
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

#include <string>

namespace ub_tester {

std::string getExprAsString(const clang::Expr*, const clang::ASTContext*);
std::string getExprLineNCol(const clang::Expr*, const clang::ASTContext*);
clang::QualType getLowestLevelPointeeType(clang::QualType);

std::string getRangeAsString(
    const clang::SourceRange& Range, const clang::ASTContext* Context);

}; // namespace ub_tester
