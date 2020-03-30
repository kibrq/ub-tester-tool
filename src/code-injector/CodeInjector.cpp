#include "code-injector/CodeInjector.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace ub_tester {

CodeInjector::CodeInjector(const std::string& Filename) : CodeInjector() {
  openFile(Filename);
}

CodeInjector::~CodeInjector() { closeFile(); }

void CodeInjector::closeFile() {
  if (isClosed_)
    return;

  std::ofstream ofs(OutputFilename_, std::ofstream::out);
  for (const auto& line : FileBuffer_) {
    ofs << line << '\n';
  }

  SourceFile_.clear();
  FileBuffer_.clear();
  LineOffsets_.clear();
  ColumnOffsets_.clear();
  isClosed_ = true;
}

void CodeInjector::setOutputFilename(const std::string& OutputFilename) {
  OutputFilename_ = OutputFilename;
}

void CodeInjector::openFile(const std::string& Filename) {
  if (not isClosed_) {
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
    SourceFile_.push_back(FileBuffer_.back());
  }
  LineOffsets_.resize(FileBuffer_.size());
  isClosed_ = false;
}

namespace {
bool isSymbol(char Ch, SPECIAL_SYMBOL Symb) {
  return Ch == static_cast<char>(Symb);
}
bool isNotSymbol(char Ch, SPECIAL_SYMBOL Symb) {
  return Ch != static_cast<char>(Symb);
}
} // namespace

bool SourcePosition::onTheSameLine(const SourcePosition& Other) {
  return Line_ == Other.Line_;
}

size_t SourcePosition::diff(const SourcePosition& Other) {
  return Other.Col_ - Col_;
}

SourcePosition SourcePosition::moveCol(size_t Val) {
  return {Line_, Col_ + Val};
}

SourcePosition SourcePosition::moveLine(size_t Val) {
  return {Line_ + Val, Col_};
}

CodeInjector::OutputPosition CodeInjector::transform(SourcePosition Pos) {
  return {
      Pos.Line_ + LineOffsets_[Pos.Line_],
      Pos.Col_ + ColumnOffsets_[Pos.Line_][Pos.Col_]};
}

char& CodeInjector::getChar(SourcePosition Pos) {
  OutputPosition OPos = transform(Pos);
  return FileBuffer_[OPos.Line_][OPos.Col_];
}

std::string& CodeInjector::getLine(SourcePosition Pos) {
  OutputPosition OPos = transform(Pos);
  return FileBuffer_[OPos.Line_];
}

void CodeInjector::changeLineOffsets(SourcePosition Pos, int Val) {
  for (size_t CurLine = Pos.Line_; CurLine < LineOffsets_.size(); CurLine++) {
    LineOffsets_[CurLine] += Val;
  }
}

void CodeInjector::changeColumnOffsets(SourcePosition BeginPos, int Val) {
  for (size_t CurCol = BeginPos.Col_;
       CurCol < ColumnOffsets_[BeginPos.Line_].size(); CurCol++) {
    ColumnOffsets_[BeginPos.Line_][CurCol] += Val;
  }
}

CodeInjector&
CodeInjector::insertLineBefore(SourcePosition Pos, const std::string& Line) {
  OutputPosition OPos = transform(Pos);
  FileBuffer_.insert(FileBuffer_.begin() + OPos.Line_, Line);
  changeLineOffsets(Pos, 1);
  return *this;
}

CodeInjector&
CodeInjector::insertLineAfter(SourcePosition Pos, const std::string& Line) {
  return insertLineBefore(Pos.moveLine(1), Line);
}

CodeInjector& CodeInjector::insertSubstringBefore(
    SourcePosition Pos, const std::string& Substring) {

  getLine(Pos).insert(transform(Pos).Col_, Substring);

  changeColumnOffsets(Pos, Substring.length());
  return *this;
}

CodeInjector& CodeInjector::insertSubstringAfter(
    SourcePosition Pos, const std::string& Substring) {
  return insertSubstringBefore(Pos.moveCol(1), Substring);
}

CodeInjector& CodeInjector::eraseLine(SourcePosition Pos) {
  OutputPosition OPos = transform(Pos);
  FileBuffer_.erase(FileBuffer_.begin() + OPos.Line_);
  changeLineOffsets(Pos, -1);
  return *this;
}

CodeInjector& CodeInjector::substituteSubstring(
    SourcePosition BeginPos, SourcePosition EndPos,
    const std::string& Substring) {
  size_t Length = BeginPos.onTheSameLine(EndPos)
                      ? BeginPos.diff(EndPos)
                      : ColumnOffsets_[BeginPos.Line_].size() - BeginPos.Col_;
  getLine(BeginPos).erase(transform(BeginPos).Col_, Length);
  getLine(BeginPos).insert(transform(BeginPos).Col_, Substring);
  changeColumnOffsets(BeginPos, Substring.length() - Length);

  if (BeginPos.onTheSameLine(EndPos)) {
    return *this;
  }

  BeginPos = BeginPos.moveLine(1);

  while (BeginPos.Line_ < EndPos.Line_) {
    eraseLine(BeginPos);
    BeginPos = BeginPos.moveLine(1);
  }

  getLine(EndPos).erase(
      transform({EndPos.Line_, 0}).Col_, transform(EndPos).Col_);

  return *this;
}

SourcePosition
CodeInjector::findFirstEntry(SourcePosition Pos, const std::string& Substring) {
  while (1) {
    size_t pos = SourceFile_[Pos.Line_].find(Substring, Pos.Col_);
    if (pos == std::string::npos) {
      Pos.Col_ = 0;
      Pos = Pos.moveLine(1);
    } else {
      Pos.Col_ = pos;
      break;
    }
  }

  return Pos;
}

SourcePosition CodeInjector::findFirstEntry(SourcePosition Pos, char Char) {
  while (1) {
    size_t pos = SourceFile_[Pos.Line_].find_first_of(Char, Pos.Col_);
    if (pos == std::string::npos) {
      Pos.Col_ = 0;
      Pos = Pos.moveLine(1);
    } else {
      Pos.Col_ = pos;
      break;
    }
  }

  return Pos;
}

CodeInjector& CodeInjector::substitute(
    SourcePosition BeginPos, std::string SourceFormat, std::string OutputFormat,
    const std::vector<std::string>& Args) {
  SourceFormat += static_cast<char>(SPECIAL_SYMBOL::ARG);
  OutputFormat += static_cast<char>(SPECIAL_SYMBOL::ARG);
  size_t CurArg = 0;

  SourcePosition CurBegin = BeginPos, CurPos = BeginPos;
  size_t CurOutputBegin = 0, CurOutputPos = 0;
  size_t CurFormatPos = 0;

  while (1) {
    while (getChar(CurPos) == SourceFormat[CurFormatPos]) {
      CurPos = CurPos.moveCol(1), CurFormatPos++;
    }

    assert(
        isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG) ||
        isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ANY));

    if (isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ANY)) {
      CurFormatPos++;

      assert(isNotSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ANY));

      CurPos = isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG)
                   ? findFirstEntry(CurPos, Args[CurArg])
                   : findFirstEntry(CurPos, SourceFormat[CurFormatPos]);
      continue;
    }

    assert(isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG));

    while (isNotSymbol(OutputFormat[CurOutputPos], SPECIAL_SYMBOL::ARG)) {
      CurOutputPos++;
    }
    substituteSubstring(
        CurBegin, CurPos,
        OutputFormat.substr(CurOutputBegin, CurOutputPos - CurOutputBegin));
    CurOutputBegin = ++CurOutputPos;

    if (CurArg < Args.size()) {
      CurPos = findFirstEntry(CurPos, Args[CurArg]);
      for (const char& ArgChar : Args[CurArg]) {
        if (ArgChar == '\n') {
          CurPos.moveLine(1);
          CurPos.Col_ = 0;
        } else {
          assert(getChar(CurPos) == ArgChar);
          CurPos = CurPos.moveCol(1);
        }
      }
      CurFormatPos++;
      if (isNotSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG) &&
          isNotSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ANY)) {
        CurPos = findFirstEntry(CurPos, SourceFormat[CurFormatPos]);
      }
      CurBegin = CurPos;
      CurArg++;
    } else {
      break;
    }
  }

  return *this;
}
} // namespace ub_tester
