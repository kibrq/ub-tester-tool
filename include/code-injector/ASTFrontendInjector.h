#pragma once

#include "clang/AST/ASTContext.h"

#include "code-injector/CodeInjector.h"

#include <memory>
#include <vector>

namespace ub_tester {

class ASTFrontendInjector {
public:
  static ASTFrontendInjector& getInstance();

  ASTFrontendInjector(const ASTFrontendInjector& other) = delete;
  ASTFrontendInjector(ASTFrontendInjector&& other) = delete;
  ASTFrontendInjector& operator=(const ASTFrontendInjector& other) = delete;
  ASTFrontendInjector& operator=(ASTFrontendInjector&& other) = delete;

  void addFile(const clang::ASTContext* Context);
  void addFile(const std::string& Filename);

  // Formats must have equal count of %.
  // substitute(Context, Loc, "%+%", "AssertSum(%, %)", "a", Expr);

  void applySubstitutions();

  void substitute(const clang::ASTContext* Context,
                  const clang::SourceRange& Range, std::string NewString);

  void substitute(const clang::ASTContext* Context,
                  const clang::SourceLocation& BeginLoc,
                  std::string SourceFormat, std::string SubstitutionFormat,
                  const code_injector::SubArgs& Args);

  template <typename... Args>
  void substitute(const clang::ASTContext* Context,
                  const clang::SourceLocation& BeginLoc,
                  std::string SourceFormat, std::string SubstitutionFormat,
                  Args... as);

private:
  explicit ASTFrontendInjector() = default;

private:
  std::vector<std::unique_ptr<code_injector::CodeInjector>> Injectors;
};

}; // namespace ub_tester

#include "ASTFrontendInjectorImpl.hpp"
