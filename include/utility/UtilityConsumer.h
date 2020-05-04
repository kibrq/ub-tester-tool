#pragma once

#include "clang/AST/ASTConsumer.h"
#include "GetFuncCodeAvailVisitor.h"

namespace ub_tester {

class UtilityConsumer : public clang::ASTConsumer {
public:
  explicit UtilityConsumer(clang::ASTContext* Context);

  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  GetFuncCodeAvailVisitor FuncCodeAvailVisitor;
};

} // namespace ub_tester