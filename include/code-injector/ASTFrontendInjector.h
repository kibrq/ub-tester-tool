#pragma once

#include "clang/AST/ASTContext.h"

#include "code-injector/CodeInjector.h"

#include <memory>
#include <vector>

namespace ub_tester {

using SubArgs = std::vector<std::string>;

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
  void substitute(const clang::ASTContext* Context,
                  const clang::SourceLocation& BeginLoc,
                  std::string_view SourceFormat,
                  std::string_view SubstitutionFormat, const SubArgs& Args);

  template <typename... Args>
  void substitute(const clang::ASTContext* Context,
                  const clang::SourceLocation& BeginLoc,
                  std::string_view SourceFormat,
                  std::string_view SubstitutionFormat, Args... as);

private:
  explicit ASTFrontendInjector() = default;

private:
  std::unique_ptr<code_injector::CodeInjector> Injector;
};

}; // namespace ub_tester

#include "ASTFrontendInjectorImpl.hpp"
