#pragma once

#include "UBUtility.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {
class PointerSubstituter
    : public clang::RecursiveASTVisitor<PointerSubstituter> {
public:
  explicit PointerSubstituter(clang::ASTContext* Context);
  bool VisitFunctionDecl(clang::FunctionDecl* fd);
  bool VisitVarDecl(clang::VarDecl* vd);
  bool VisitBinaryOperator(clang::BinaryOperator* bo);

private:
  clang::ASTContext* Context;
  var_info_ pointer_;
};
} // namespace ub_tester
