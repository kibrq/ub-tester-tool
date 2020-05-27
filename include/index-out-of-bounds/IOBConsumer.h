#pragma once

#include "index-out-of-bounds/CArrayHandler.h"
#include "clang/AST/ASTConsumer.h"
#include <string>

namespace ub_tester {

class IOBConsumer : public clang::ASTConsumer {
public:
  explicit IOBConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  CArrayVisitor ArrayVisitor_;
};

} // namespace ub_tester
