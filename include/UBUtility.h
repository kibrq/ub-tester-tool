#pragma once
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

#include <string>
#include <unordered_set>

namespace ub_tester {

std::string getExprAsString(const clang::Expr*, const clang::ASTContext*);
std::string getExprLineNCol(const clang::Expr*, const clang::ASTContext*);
clang::QualType getLowestLevelPointeeType(clang::QualType);

std::string getRangeAsString(const clang::SourceRange& Range, const clang::ASTContext* Context);

clang::SourceLocation getNameLastLoc(const clang::DeclaratorDecl*, const clang::ASTContext*);
clang::SourceLocation getAfterNameLoc(const clang::DeclaratorDecl*, const clang::ASTContext*);

std::string getFuncNameWithArgsAsString(const clang::FunctionDecl* FuncDecl);

namespace func_code_avail {

bool hasFuncAvailCode(const clang::FunctionDecl* FuncDecl);
void setHasFuncAvailCode(const clang::FunctionDecl* FuncDecl);

} // namespace func_code_avail

}; // namespace ub_tester
