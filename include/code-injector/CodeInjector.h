#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

namespace ub_tester {

class CodeInjector {
public:
  explicit CodeInjector() = default;

  explicit CodeInjector(const std::string& Filename);

  CodeInjector(const CodeInjector& other) = delete;

  CodeInjector& operator=(const CodeInjector& other) = delete;

  CodeInjector(CodeInjector&& other) = delete;

  CodeInjector& operator=(CodeInjector&& other) = delete;

  ~CodeInjector();

  void openFile(const std::string& Filename);

  void closeFile(const std::string& Filename);

  CodeInjector& eraseSubstring(size_t LineNum, size_t BeginPos, size_t Length);

  CodeInjector& insertSubstringAfter(
      size_t LineNum, size_t BeginPos, const std::string& Substring);

  CodeInjector& insertSubstringBefore(
      size_t LineNum, size_t BeginPos, const std::string& Substring);

  CodeInjector& insertLineAfter(size_t LineNum, const std::string& Line);

  CodeInjector& insertLineBefore(size_t LineNum, const std::string& Line);

  CodeInjector& substituteSubline(
      size_t LineNum, size_t BeginPos, size_t Length,
      const std::string& Substr);

  CodeInjector& substitute(
      size_t BeginLine, size_t BeginPos, const std::string& SourceFormat,
      const std::string& OutputFormat, const std::vector<std::string>& Args);

private:
  char get(size_t LineNum, size_t ColumnNum);
  size_t transformLineNum(size_t LineNum);
  size_t transformColumnNum(size_t LineNum, size_t ColumnNum);
  void changeLineOffsets(size_t InitPos, int Val = 1);
  void changeColumnOffsets(size_t LineNum, size_t InitPos, int Val = 1);

private:
  bool Closed_;
  std::vector<std::string> FileBuffer_;
  std::vector<int> LineOffsets_;
  std::vector<std::vector<int>> ColumnOffsets_;
};

} // namespace ub_tester
