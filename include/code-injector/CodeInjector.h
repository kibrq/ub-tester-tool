#pragma once

#include "clang/Basic/SourceManager.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ub_tester {

class CodeInjector {
public:
  static CodeInjector& getInstance();

  CodeInjector(const CodeInjector& other) = delete;
  CodeInjector& operator=(const CodeInjector& other) = delete;
  CodeInjector(CodeInjector&& other) = delete;
  CodeInjector& operator=(CodeInjector&& other) = delete;
  ~CodeInjector();

  void openFile(const clang::SourceManager* SM);
  void openFile(const std::string& Filename);

  void closeFile();

  CodeInjector& substituteInOneLine(
      size_t LineNum, size_t Begin, size_t End,
      const std::string& Substitution);

  CodeInjector& substitute(
      size_t LineBegin, size_t ColumnBegin, size_t LineEnd, size_t ColumnEnd,
      const std::string& Substitution);

  CodeInjector& substitute(
      const clang::SourceLocation& Begin, const clang::SourceLocation& End,
      const std::string& Substitution);

  CodeInjector& insertLineAfter(size_t LineNum, const std::string& Line);

  CodeInjector& insertLineBefore(size_t LineNum, const std::string& Line);

  CodeInjector&
  insertLineAfter(const clang::SourceLocation& Loc, const std::string& Line);

  CodeInjector&
  insertLineBefore(const clang::SourceLocation& Loc, const std::string& Line);

private:
  explicit CodeInjector();
  void increaseLineOffsets(size_t InitPos, size_t Val = 1);
  void increaseColumnOffsets(size_t LineNum, size_t InitPos, size_t Val = 1);

private:
  bool Closed_ = true;
  const clang::SourceManager* SM_;
  std::vector<std::string> FileBuffer_;
  std::vector<size_t> LineOffsets_;
  std::vector<std::vector<size_t>> ColumnOffsets_;
};

} // namespace ub_tester
