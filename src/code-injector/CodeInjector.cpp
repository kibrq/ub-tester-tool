#include "code-injector/CodeInjector.h"
#include "clang/Basic/SourceManager.h"

#include <cassert>
#include <fstream>
#include <sstream>

using namespace clang;

namespace ub_tester {

CodeInjector& CodeInjector::getInstance() {
  static CodeInjector Instance_;
  return Instance_;
}

CodeInjector::CodeInjector() {}
CodeInjector::~CodeInjector() { closeFile(); }

std::string generateOutputFilename() { return "a.out"; }

void CodeInjector::closeFile() {
  if (Closed_)
    return;

  std::ofstream ofs(generateOutputFilename(), std::ofstream::out);
  for (const auto& line : FileBuffer_) {
    ofs << line << '\n';
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

void CodeInjector::openFile(const SourceManager* SM) {
  SM_ = SM;
  std::string filename =
      SM_->getFilename(SM_->getLocForStartOfFile(SM_->getMainFileID())).str();
  openFile(filename);
}

void CodeInjector::increaseLineOffsets(size_t InitPos, size_t Val) {
  for (size_t Pos = InitPos; Pos < LineOffsets_.size(); Pos++) {
    LineOffsets_[Pos] += Val;
  }
}

void CodeInjector::increaseColumnOffsets(
    size_t LineNum, size_t InitPos, size_t Val) {
  for (size_t Pos = InitPos; Pos < ColumnOffsets_[LineNum].size(); Pos++) {
    ColumnOffsets_[LineNum][Pos] += Val;
  }
}

CodeInjector&
CodeInjector::insertLineBefore(size_t LineNum, const std::string& Line) {
  size_t NewLineNum = LineNum + LineOffsets_[LineNum];
  FileBuffer_.insert(FileBuffer_.begin() + NewLineNum, Line);
  increaseLineOffsets(LineNum);
  return *this;
}

CodeInjector&
CodeInjector::insertLineAfter(size_t LineNum, const std::string& Line) {
  return insertLineBefore(LineNum + 1, Line);
}

CodeInjector& CodeInjector::insertLineBefore(
    const SourceLocation& Loc, const std::string& Line) {
  return insertLineBefore(SM_->getSpellingLineNumber(Loc), Line);
}

CodeInjector& CodeInjector::insertLineAfter(
    const SourceLocation& Loc, const std::string& Line) {
  return insertLineAfter(SM_->getSpellingLineNumber(Loc), Line);
}

CodeInjector& CodeInjector::substituteInOneLine(
    size_t LineNum, size_t Begin, size_t End, const std::string& Substitution) {
  size_t NewLineNum = LineNum + LineOffsets_[LineNum];
  size_t NewBegin = Begin + ColumnOffsets_[LineNum][Begin];
  size_t NewEnd = End + ColumnOffsets_[LineNum][End];
  FileBuffer_[NewLineNum].erase(NewBegin, End - Begin);
  FileBuffer_[NewLineNum].insert(NewBegin, Substitution);
  increaseColumnOffsets(LineNum, Begin, Substitution.length() - (End - Begin));
  return *this;
}

CodeInjector& CodeInjector::substitute(
    size_t LineBegin, size_t ColumnBegin, size_t LineEnd, size_t ColumnEnd,
    const std::string& Substitution) {

  assert(LineBegin <= LineEnd);

  if (LineBegin == LineEnd) {
    return substituteInOneLine(LineBegin, ColumnBegin, ColumnEnd, Substitution);
  }

  std::stringstream SS(Substitution);

  for (size_t LineNum = LineBegin; LineNum <= LineEnd; LineNum++) {
    std::string Line;
    assert(!SS.eof());
    std::getline(SS, Line);
    if (LineNum != LineBegin && LineNum != LineEnd) {
      substituteInOneLine(LineNum, 0, FileBuffer_[LineNum].length(), Line);
    } else if (LineNum == LineBegin) {
      substituteInOneLine(
          LineNum, ColumnBegin, FileBuffer_[LineNum].length(), Line);
    } else {
      substituteInOneLine(LineNum, 0, ColumnEnd, Line);
    }
  }
  return *this;
}

CodeInjector& CodeInjector::substitute(
    const SourceLocation& Begin, const SourceLocation& End,
    const std::string& Substitution) {

  size_t LineBegin = SM_->getSpellingLineNumber(Begin);
  size_t LineEnd = SM_->getSpellingLineNumber(End) + 1;
  size_t ColumnBegin = SM_->getSpellingColumnNumber(Begin);
  size_t ColumnEnd = SM_->getSpellingColumnNumber(End) + 1;

  return substitute(LineBegin, ColumnBegin, LineEnd, ColumnEnd, Substitution);
}

} // namespace ub_tester
