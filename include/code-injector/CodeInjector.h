#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ub_tester {

enum class SPECIAL_SYMBOL : char { ARG = '@', ANY = '#' };

struct SourcePosition {
  size_t Line_, Col_;
  bool onTheSameLine(const SourcePosition& Other);
  size_t diff(const SourcePosition& Other);
  SourcePosition moveCol(size_t Val);
  SourcePosition moveLine(size_t Val);
};

class CodeInjector {
public:
  explicit CodeInjector() = default;

  explicit CodeInjector(const std::string& Filename);

  CodeInjector(const CodeInjector& other) = delete;

  CodeInjector& operator=(const CodeInjector& other) = delete;

  CodeInjector(CodeInjector&& other) = delete;

  CodeInjector& operator=(CodeInjector&& other) = delete;

  ~CodeInjector();

  void setOutputFilename(const std::string& OutputFilename);

  void openFile(const std::string& Filename);

  void closeFile();

  CodeInjector& eraseLine(SourcePosition BeginPos);

  CodeInjector&
  insertSubstringAfter(SourcePosition Pos, const std::string& Substring);

  CodeInjector&
  insertSubstringBefore(SourcePosition Pos, const std::string& Substring);

  CodeInjector& insertLineAfter(SourcePosition Pos, const std::string& Line);

  CodeInjector& insertLineBefore(SourcePosition Pos, const std::string& Line);

  CodeInjector& substituteSubstring(
      SourcePosition BeginPos, SourcePosition EndPos,
      const std::string& Substring);

  CodeInjector& substitute(
      SourcePosition Pos, std::string SourceFormat, std::string OutputFormat,
      const std::vector<std::string>& Args);

private:
  struct OutputPosition {
    size_t Line_, Col_;
  };

private:
  char& getChar(SourcePosition Pos);
  std::string& getLine(SourcePosition Pos);
  OutputPosition transform(SourcePosition Pos);
  void changeLineOffsets(SourcePosition BeginPos, int Val = 1);
  void changeColumnOffsets(SourcePosition BeginPos, int Val = 1);
  SourcePosition
  findFirstEntry(SourcePosition BeginPos, const std::string& Substring);
  SourcePosition findFirstEntry(SourcePosition BeginPos, char Char);

private:
  bool isClosed_;
  std::string OutputFilename_;
  std::vector<std::string> FileBuffer_, SourceFile_;
  std::vector<int> LineOffsets_;
  std::vector<std::vector<int>> ColumnOffsets_;
};

} // namespace ub_tester
