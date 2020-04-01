#pragma once

#include "clang/AST/ASTContext.h"

#include "code-injector/CodeInjector.h"

#include <unordered_map>
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

  void insertLineBefore(
      const clang::ASTContext* Context, const clang::SourceLocation& Loc,
      const std::string& Line);

  void insertLineAfter(
      const clang::ASTContext* Context, const clang::SourceLocation& Loc,
      const std::string& Line);

  // Formats must have equal count of %.
  // substitute(Context, Loc, "%+%", "AssertSum(%, %)", "a", Expr);
  void substitute(
      const clang::ASTContext* Context, const clang::SourceLocation& BeginLoc,
      const std::string& SourceFormat, const std::string& SubstitutionFormat,
      const SubArgs& Args);

  template <typename... Args>
  void substitute(
      const clang::ASTContext* Context, clang::SourceLocation& BeginLoc,
      const std::string& SourceFormat, const std::string& SubstitutionFormat,
      Args... as);

  // These functions suppose that this range contains on one Line and you won't
  // handle this range anymore

  void substituteSubstring(
      const clang::ASTContext* Context, const clang::SourceLocation& Begin,
      const clang::SourceLocation& End, const std::string& Substitution);

  void substituteSubstring(
      const clang::ASTContext* Context, const clang::CharSourceRange& Range,
      const std::string& Substitution);

  void substituteSubstring(
      const clang::ASTContext* Context, const clang::SourceRange& Range,
      const std::string& Substitution);

private:
  explicit ASTFrontendInjector() = default;

private:
  std::unordered_map<std::string, CodeInjector> Files_;
};

}; // namespace ub_tester

#include "ASTFrontendInjectorImpl.hpp"
