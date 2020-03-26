#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class FindArithmeticOverflowVisitor
    : public RecursiveASTVisitor<FindArithmeticOverflowVisitor> {
public:
  explicit FindArithmeticOverflowVisitor(ASTContext* Context);

  bool VisitBinaryOperator(BinaryOperator* Binop);

private:
  ASTContext* Context;
};

} // namespace ub_tester
