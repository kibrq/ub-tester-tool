#pragma once

#include "clang/AST/ASTContext.h"

#include "code-injector/CodeInjector.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace ub_tester {

class ASTFrontendInjector {
public:
  static ASTFrontendInjector& getInstance();
  static void initialize(const std::vector<std::string>& SourcePathList);

  ASTFrontendInjector(const ASTFrontendInjector& other) = delete;
  ASTFrontendInjector(ASTFrontendInjector&& other) = delete;
  ASTFrontendInjector& operator=(const ASTFrontendInjector& other) = delete;
  ASTFrontendInjector& operator=(ASTFrontendInjector&& other) = delete;

  void substituteIncludePaths();

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
  void addFile(const std::string& Filename);

private:
  explicit ASTFrontendInjector() = default;
  static std::unique_ptr<ASTFrontendInjector> Instance_;

private:
  std::unordered_map<std::string, std::unique_ptr<code_injector::CodeInjector>>
      Injectors_;
};

}; // namespace ub_tester

#include "ASTFrontendInjectorImpl.hpp"
