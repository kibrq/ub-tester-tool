#pragma once

#include "arithmetic-ub/FindArithmeticUBVisitor.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class FindArithmeticUBConsumer : public clang::ASTConsumer {
public:
  explicit FindArithmeticUBConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindArithmeticUBVisitor Visitor_;
};

} // namespace ub_tester
