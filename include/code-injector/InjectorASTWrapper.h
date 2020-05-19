#pragma once

#include "code-injector/CodeInjector.h"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include <memory>
#include <vector>

using ub_tester::code_injector::SubstPriorityKind;

namespace ub_tester {

class InjectorASTWrapper {
public:
  static InjectorASTWrapper& getInstance();

  InjectorASTWrapper(const InjectorASTWrapper& other) = delete;
  InjectorASTWrapper(InjectorASTWrapper&& other) = delete;
  InjectorASTWrapper& operator=(const InjectorASTWrapper& other) = delete;
  InjectorASTWrapper& operator=(InjectorASTWrapper&& other) = delete;

  void addFile(const clang::ASTContext* Context);

  void substituteIncludePaths(const std::vector<std::string>&);

  void applySubstitutions();

  void substitute(const clang::SourceRange& Range, std::string NewString,
                  const clang::ASTContext* Context);

  void substitute(code_injector::Substitution&& Subst,
                  const clang::ASTContext* Context);

private:
  explicit InjectorASTWrapper() = default;

private:
  std::vector<std::unique_ptr<code_injector::CodeInjector>> Injectors_;
};

template <typename... Expr>
code_injector::SubstArgs createSubstArgs(const clang::ASTContext* Context,
                                         Expr... Exprs);

struct SubstitutionASTWrapper {
  SubstitutionASTWrapper(const clang::ASTContext* Context)
      : Context_{Context} {}
  inline SubstitutionASTWrapper& setLoc(clang::SourceLocation Loc) {
    Subst_.setOffset(Context_->getSourceManager().getFileOffset(Loc));
    return *this;
  }
  inline SubstitutionASTWrapper& setPrior(SubstPriorityKind Prior) {
    Subst_.setPrior(Prior);
    return *this;
  }
  inline SubstitutionASTWrapper& setFormats(std::string SourceFormat,
                                            std::string OutputFormat) {
    Subst_.setSourceFormat(std::move(SourceFormat));
    Subst_.setOutputFormat(std::move(OutputFormat));
    return *this;
  }
  template <typename... Expr>
  inline SubstitutionASTWrapper& setArguments(Expr... Exprs) {
    Subst_.setArguments(createSubstArgs(Context_, Exprs...));
    return *this;
  }

  inline void apply() {
    InjectorASTWrapper::getInstance().substitute(std::move(Subst_), Context_);
  }

private:
  const clang::ASTContext* Context_;
  code_injector::Substitution Subst_;
};

}; // namespace ub_tester

#include "InjectorASTWrapperImpl.hpp"
