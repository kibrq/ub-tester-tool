#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class FindArithmeticUBVisitor
    : public clang::RecursiveASTVisitor<FindArithmeticUBVisitor> {
public:
  explicit FindArithmeticUBVisitor(clang::ASTContext* Context);

  bool VisitBinaryOperator(clang::BinaryOperator* Binop);
  bool VisitUnaryOperator(clang::UnaryOperator* Unop);
  bool VisitCompoundAssignOperator(clang::CompoundAssignOperator* CompAssignOp);

private:
  clang::ASTContext* Context;
};

} // namespace ub_tester
