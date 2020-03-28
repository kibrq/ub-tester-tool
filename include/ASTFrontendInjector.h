#pragma once

#include "clang/Basic/SourceManager.h"

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

  void addFile(const clang::SourceManager& SM);

  void insertLineBefore(
      const clang::SourceManager& SM, const clang::SourceLocation& Loc,
      const std::string& Line);

  void insertLineAfter(
      const clang::SourceManager& SM, const clang::SourceLocation& Loc,
      const std::string& Line);

  // Formats must have equal count of %.
  // substitute(Loc, "%+%", "AssertSum(%, %)");
  void substitute(
      const clang::SourceManager& SM, const clang::SourceLocation& BeginLoc,
      const std::string& SourceFormat, const std::string& SubstitutionFormat,
      const SubArgs& Args);

private:
  explicit ASTFrontendInjector() = default;

private:
  std::unordered_map<std::string, CodeInjector> Files_;
};

}; // namespace ub_tester
