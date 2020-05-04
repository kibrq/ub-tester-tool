#pragma once

#include "clang/AST/ASTConsumer.h"

#include "type-substituter/TypeSubstituterVisitor.h"

namespace ub_tester {

class TypeSubstituterConsumer {
public:
  TypeSubstituterConsumer(clang::ASTContext*);
  virtual void HandleTranslationUnit(clang::ASTContext&);

private:
  TypeSubstituterVisitor Substituter_;
};

} // namespace ub_tester
