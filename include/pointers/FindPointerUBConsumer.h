#pragma once

#include "pointers/FindPointerUBVisitor.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class FindPointerUBConsumer : public clang::ASTConsumer {
public:
  explicit FindPointerUBConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindPointerUBVisitor FindPointerUBVisitor_;
};

} // namespace ub_tester
