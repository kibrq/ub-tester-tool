#pragma once

#include "clang/Basic/SourceManager.h"

#include "code-injector/CodeInjector.h"

#include <unordered_map>
#include <vector>

namespace ub_tester {

class ASTFrontendInjector {
public:
  void addFile(const clang::SourceManager* SM);

  void
  insertLineBefore(const clang::SourceLocation& Loc, const std::string& Line);
  void
  insertLineAfter(const clang::SourceLocation& Loc, const std::string& Line);

  // Formats must have equal count of %.
  // substitute(Loc, "%s+%s", "AssertSum(%s,%s)");
  void substitute(
      const clang::SourceLocation& BeginLoc, const std::string& SourceFormat,
      const std::string& SubstitutedFormat, std::vector<std::string> Args);

private:
  const clang::SourceManager* SM_;
  std::unordered_map<std::string, CodeInjector> Files_;
};

}; // namespace ub_tester
