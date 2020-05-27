#pragma once

#include "pointers/PointerHandler.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class PointersConsumer : public clang::ASTConsumer {
public:
  explicit PointersConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  PointerVisitor PointerVisitor_;
};

} // namespace ub_tester
