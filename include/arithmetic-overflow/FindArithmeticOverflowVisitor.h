#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class FindArithmeticOverflowVisitor
    : public clang::RecursiveASTVisitor<FindArithmeticOverflowVisitor> {
public:
  explicit FindArithmeticOverflowVisitor(clang::ASTContext* Context);

  bool VisitBinaryOperator(clang::BinaryOperator* Binop);

private:
  clang::ASTContext* Context;
};

} // namespace ub_tester
