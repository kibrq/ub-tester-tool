#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {
class TypeSubstituterVisitor
    : public clang::RecursiveASTVisitor<TypeSubstituterVisitor> {
public:
  explicit TypeSubstituterVisitor(clang::ASTContext*);

  bool VisitValueDecl(clang::ValueDecl*);

private:
  clang::ASTContext* Context_;
};
} // namespace ub_tester
