#pragma once

#include "clang/AST/ASTConsumer.h"

#include "type-substituter/TypeSubstituterVisitor.h"

namespace ub_tester {

class TypeSubstituterVisitor;
class TypeSubstituterConsumer : public clang::ASTConsumer {
public:
  TypeSubstituterConsumer(clang::ASTContext*);
  virtual void HandleTranslationUnit(clang::ASTContext&);

private:
  TypeSubstituterVisitor Substituter_;
};

} // namespace ub_tester
