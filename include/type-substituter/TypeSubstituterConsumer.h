#pragma once

#include "type-substituter/TypeSubstituterVisitor.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class TypeSubstituterConsumer : public clang::ASTConsumer {
public:
  TypeSubstituterConsumer(clang::ASTContext*);
  virtual void HandleTranslationUnit(clang::ASTContext&);

private:
  TypeSubstituterVisitor Substituter_;
};

} // namespace ub_tester
