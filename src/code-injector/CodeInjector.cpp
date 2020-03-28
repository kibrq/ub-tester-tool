#include "code-injector/CodeInjector.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace ub_tester {

CodeInjector::CodeInjector(const std::string& Filename) : CodeInjector() {
  openFile(Filename);
}

void CodeInjector::setOutputFilename(std::string OutputFilename) {
  OutputFilename_ = OutputFilename;
}

CodeInjector::~CodeInjector() { closeFile(); }

void CodeInjector::closeFile() {
  if (Closed_)
    return;

  std::ofstream ofs(OutputFilename_, std::ofstream::out);

  for (const auto& line : FileBuffer_) {
    ofs << line << std::endl;
  }

  FileBuffer_.clear();
  LineOffsets_.clear();
  ColumnOffsets_.clear();
  Closed_ = true;
}

void CodeInjector::openFile(const std::string& Filename) {
  if (!Closed_) {
    closeFile();
  }
  std::ifstream ins(Filename, std::ifstream::in);
  if (!ins.good()) {
    // TODO
  }
  while (!ins.eof()) {
    std::string inp;
    std::getline(ins, inp);
    ColumnOffsets_.emplace_back(inp.length(), 0);
    FileBuffer_.emplace_back(std::move(inp));
  }
  LineOffsets_.resize(FileBuffer_.size());
  Closed_ = false;
}

char CodeInjector::get(size_t LineNum, size_t ColumnNum) {
  return FileBuffer_[transformLineNum(LineNum)]
                    [transformColumnNum(LineNum, ColumnNum)];
}

size_t CodeInjector::transformLineNum(size_t LineNum) {
  return LineNum + LineOffsets_[LineNum];
}

size_t CodeInjector::transformColumnNum(size_t LineNum, size_t ColumnNum) {
  return ColumnNum + ColumnOffsets_[LineNum][ColumnNum];
}

void CodeInjector::changeLineOffsets(size_t InitPos, int Val) {
  for (size_t Pos = InitPos; Pos < LineOffsets_.size(); Pos++) {
    LineOffsets_[Pos] += Val;
  }
}

void CodeInjector::changeColumnOffsets(
    size_t LineNum, size_t InitPos, int Val) {
  for (size_t Pos = InitPos; Pos < ColumnOffsets_[LineNum].size(); Pos++) {
    ColumnOffsets_[LineNum][Pos] += Val;
  }
}

CodeInjector&
CodeInjector::eraseSubstring(size_t LineNum, size_t BeginPos, size_t Length) {

  FileBuffer_[transformLineNum(LineNum)].erase(
      transformColumnNum(LineNum, BeginPos), Length);

  changeColumnOffsets(LineNum, BeginPos + Length, -Length);
  return *this;
}

CodeInjector&
CodeInjector::insertLineBefore(size_t LineNum, const std::string& Line) {
  FileBuffer_.insert(FileBuffer_.begin() + transformLineNum(LineNum), Line);
  changeLineOffsets(LineNum, 1);
  return *this;
}

CodeInjector&
CodeInjector::insertLineAfter(size_t LineNum, const std::string& Line) {
  return insertLineBefore(LineNum + 1, Line);
}

CodeInjector& CodeInjector::insertSubstringBefore(
    size_t LineNum, size_t BeginPos, const std::string& Substring) {

  FileBuffer_[transformLineNum(LineNum)].insert(
      transformColumnNum(LineNum, BeginPos), Substring);

  changeColumnOffsets(LineNum, BeginPos, Substring.length());
  return *this;
}

CodeInjector& CodeInjector::insertSubstringAfter(
    size_t LineNum, size_t BeginPos, const std::string& Substring) {
  return insertSubstringBefore(LineNum, BeginPos + 1, Substring);
}

CodeInjector& CodeInjector::substituteSubstring(
    size_t LineNum, size_t BeginPos, size_t Length,
    const std::string& Substring) {
  return eraseSubstring(LineNum, BeginPos, Length)
      .insertSubstringBefore(LineNum, BeginPos, Substring);
}

void CodeInjector::findFirstEntry(size_t& LineNum, size_t& CurPos, char Char) {
  while (get(LineNum, CurPos) != Char) {
    CurPos++;
    if (CurPos >= ColumnOffsets_[LineNum].size()) {
      CurPos = 0, LineNum++;
    }
  }
}

CodeInjector& CodeInjector::substitute(
    size_t BeginLine, size_t BeginPos, std::string SourceFormat,
    std::string OutputFormat, const std::vector<std::string>& Args) {
  SourceFormat += '%';
  OutputFormat += '%';
  size_t CurArg = 0;
  size_t CurSourceBegin = BeginPos, CurSourcePos = BeginPos;
  size_t CurOutputBegin = 0, CurOutputPos = 0;
  size_t CurFormatPos = 0;

  while (1) {
    while (get(BeginLine, CurSourcePos) == SourceFormat[CurFormatPos]) {
      CurSourcePos++, CurFormatPos++;
    }
    assert(SourceFormat[CurFormatPos] == '%');
    while (OutputFormat[CurOutputPos] != '%') {
      CurOutputPos++;
    }
    substituteSubstring(
        BeginLine, CurSourceBegin, CurSourcePos - CurSourceBegin,
        OutputFormat.substr(CurOutputBegin, CurOutputPos - CurOutputBegin));
    CurOutputBegin = ++CurOutputPos;

    if (CurArg < Args.size()) {
      findFirstEntry(BeginLine, CurSourcePos, Args[CurArg][0]);
      for (const char& ArgChar : Args[CurArg]) {
        if (ArgChar == '\n') {
          CurSourcePos = 0, BeginLine++;
        } else {
          assert(get(BeginLine, CurSourcePos) == ArgChar);
          CurSourcePos++;
        }
      }
      CurFormatPos++;
      if (SourceFormat[CurFormatPos] != '%') {
        findFirstEntry(BeginLine, CurSourcePos, SourceFormat[CurFormatPos]);
      }
      CurSourceBegin = CurSourcePos;
      CurArg++;
    } else {
      break;
    }
  }

  return *this;
}
} // namespace ub_tester
