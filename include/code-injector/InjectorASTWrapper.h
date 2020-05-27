#pragma once

#include "code-injector/CodeInjector.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include <memory>
#include <vector>

namespace ub_tester::code_injector::wrapper {

class InjectorASTWrapper {
public:
  static InjectorASTWrapper& getInstance();

  InjectorASTWrapper(const InjectorASTWrapper& other) = delete;
  InjectorASTWrapper& operator=(const InjectorASTWrapper& other) = delete;

  void addFile(const clang::ASTContext* Context);
  void substituteIncludePaths(const std::vector<std::string>&);
  void applySubstitutions();
  void substitute(const clang::SourceRange& Range, std::string NewString,
                  const clang::ASTContext* Context);
  void substitute(Substitution Subst, const clang::ASTContext* Context);

private:
  explicit InjectorASTWrapper() = default;

private:
  std::vector<std::unique_ptr<CodeInjector>> InternalInjectors_;
};

template <typename... ExprTypes>
SubstArgs createSubstArgs(const clang::ASTContext* Context, ExprTypes... Exprs);

struct SubstitutionASTWrapper {
  SubstitutionASTWrapper(const clang::ASTContext* Context)
      : Context_{Context} {}

  SubstitutionASTWrapper& setLoc(clang::SourceLocation Loc) {
    Subst_.setOffset(Context_->getSourceManager().getFileOffset(Loc));
    return *this;
  }

  SubstitutionASTWrapper& setPrior(SubstPriorityKind Prior) {
    Subst_.setPrior(Prior);
    return *this;
  }

  SubstitutionASTWrapper& setFormats(std::string SourceFormat,
                                     std::string OutputFormat) {
    Subst_.setSourceFormat(std::move(SourceFormat));
    Subst_.setOutputFormat(std::move(OutputFormat));
    return *this;
  }

  template <typename... ExprTypes>
  SubstitutionASTWrapper& setArguments(ExprTypes... Exprs) {
    Subst_.setArguments(createSubstArgs(Context_, Exprs...));
    return *this;
  }

  void apply() {
    InjectorASTWrapper::getInstance().substitute(std::move(Subst_), Context_);
  }

private:
  const clang::ASTContext* Context_;
  Substitution Subst_;
};

}; // namespace ub_tester::code_injector::wrapper

#include "InjectorASTWrapperImpl.hpp"
