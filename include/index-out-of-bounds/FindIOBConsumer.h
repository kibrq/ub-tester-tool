#pragma once

#include "index-out-of-bounds/CArrayVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include <string>

namespace ub_tester {

class FindIOBConsumer : public clang::ASTConsumer {
public:
  explicit FindIOBConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  CArrayVisitor ArrayVisitor_;
};

} // namespace ub_tester
