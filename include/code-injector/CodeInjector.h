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

  void setOutputFilename(std::string OutputFilename);

  CodeInjector(const CodeInjector& other) = delete;

  CodeInjector& operator=(const CodeInjector& other) = delete;

  CodeInjector(CodeInjector&& other) = delete;

  CodeInjector& operator=(CodeInjector&& other) = delete;

  ~CodeInjector();

  void openFile(const std::string& Filename);

  void closeFile();

  CodeInjector& insertSubstringAfter(
      size_t LineNum, size_t BeginPos, const std::string& Substring);

  CodeInjector& insertSubstringBefore(
      size_t LineNum, size_t BeginPos, const std::string& Substring);

  CodeInjector& insertLineAfter(size_t LineNum, const std::string& Line);

  CodeInjector& insertLineBefore(size_t LineNum, const std::string& Line);

  CodeInjector& substituteSubstring(
      size_t LineNum, size_t BeginPos, size_t Length,
      const std::string& Substring);

  using SubArgs = std::vector<std::string>;

  CodeInjector& substitute(
      size_t BeginLine, size_t BeginPos, std::string SourceFormat,
      std::string OutputFormat, const std::vector<std::string>& Args);

private:
  CodeInjector& eraseSubstring(size_t LineNum, size_t BeginPos, size_t Length);
  char get(size_t LineNum, size_t ColumnNum);
  size_t transformLineNum(size_t LineNum);
  size_t transformColumnNum(size_t LineNum, size_t ColumnNum);
  void changeLineOffsets(size_t InitPos, int Val = 1);
  void changeColumnOffsets(size_t LineNum, size_t InitPos, int Val = 1);
  void findFirstEntry(size_t& LineNum, size_t& CurPos, char Char);

private:
  bool Closed_;
  std::string OutputFilename_;
  std::vector<std::string> FileBuffer_;
  std::vector<int> LineOffsets_;
  std::vector<std::vector<int>> ColumnOffsets_;
};

} // namespace ub_tester
