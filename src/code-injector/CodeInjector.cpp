#include "code-injector/CodeInjector.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace ub_tester {

namespace {
bool isSymbol(char Ch, SPECIAL_SYMBOL Symb) { return Ch == static_cast<char>(Symb); }
bool isNotSymbol(char Ch, SPECIAL_SYMBOL Symb) { return Ch != static_cast<char>(Symb); }

bool isAnySymbol(char Ch) {
  for (auto S : SpecialSymbolRange::getInstance()) {
    if (isSymbol(Ch, S)) {
      return true;
    }
  }
  return false;
}
bool isNotAnySymbol(char Ch) {
  for (auto S : SpecialSymbolRange::getInstance()) {
    if (isSymbol(Ch, S)) {
      return false;
    }
  }
  return true;
}

char getAsChar(SPECIAL_SYMBOL Symb) { return static_cast<char>(Symb); }

SPECIAL_SYMBOL getAsSymb(char Ch) {
  assert(isAnySymbol(Ch));
  return static_cast<SPECIAL_SYMBOL>(Ch);
}

bool isNoneOf(char Ch, SPECIAL_SYMBOL Symb1, SPECIAL_SYMBOL Symb2) {
  return isNotSymbol(Ch, Symb1) && isNotSymbol(Ch, Symb2);
}

bool isAnyOf(char Ch, SPECIAL_SYMBOL Symb1, SPECIAL_SYMBOL Symb2) {
  return isSymbol(Ch, Symb1) || isSymbol(Ch, Symb2);
}

} // namespace

// SpecialSymbolRange

SpecialSymbolRange::Iterator::Iterator(SPECIAL_SYMBOL Symb) : Symb_(Symb) {}
SpecialSymbolRange::Iterator& SpecialSymbolRange::Iterator::operator++() {
  switch (Symb_) {
  case SPECIAL_SYMBOL::ARG:
    Symb_ = SPECIAL_SYMBOL::ANY;
    break;
  case SPECIAL_SYMBOL::ANY:
    Symb_ = SPECIAL_SYMBOL::SKIP;
    break;
  case SPECIAL_SYMBOL::SKIP:
    Symb_ = SPECIAL_SYMBOL::NONE;
    break;
  case SPECIAL_SYMBOL::NONE:
    std::abort();
    break;
  }
  return *this;
}

bool SpecialSymbolRange::Iterator::operator!=(const SpecialSymbolRange::Iterator& Other) const {
  return static_cast<char>(Symb_) != static_cast<char>(Other.Symb_);
}
SPECIAL_SYMBOL SpecialSymbolRange::Iterator::operator*() const { return Symb_; }

SpecialSymbolRange::Iterator SpecialSymbolRange::begin() const {
  return Iterator(SPECIAL_SYMBOL::ARG);
}

SpecialSymbolRange::Iterator SpecialSymbolRange::end() const {
  return Iterator(SPECIAL_SYMBOL::NONE);
}

SpecialSymbolRange& SpecialSymbolRange::getInstance() {
  static SpecialSymbolRange Range;
  return Range;
}

// CodeInjector

CodeInjector::CodeInjector(const std::string& Filename) : CodeInjector() { openFile(Filename); }

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
    SourceFile_.append(FileBuffer_.back());
    SourceFile_ += '\n';
  }
  LineOffsets_.resize(FileBuffer_.size());
  isClosed_ = false;
}

bool SourcePosition::onTheSameLine(const SourcePosition& Other) { return Line_ == Other.Line_; }

size_t SourcePosition::diff(const SourcePosition& Other) { return Other.Col_ - Col_; }

SourcePosition& SourcePosition::operator+=(const SourcePosition& Other) {
  Line_ += Other.Line_;
  Col_ += Other.Col_;
  return *this;
}

SourcePosition& SourcePosition::changeLine(size_t Line) { return *this += {Line, 0}; }

SourcePosition& SourcePosition::changeCol(size_t Col) { return *this += {0, Col}; }

CodeInjector::OutputPosition CodeInjector::transform(SourcePosition Pos) {
  return {Pos.Line_ + LineOffsets_[Pos.Line_], Pos.Col_ + ColumnOffsets_[Pos.Line_][Pos.Col_]};
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
  for (size_t CurCol = BeginPos.Col_; CurCol < ColumnOffsets_[BeginPos.Line_].size(); CurCol++) {
    ColumnOffsets_[BeginPos.Line_][CurCol] += Val;
  }
}

CodeInjector& CodeInjector::insertLineBefore(SourcePosition Pos, const std::string& Line) {
  OutputPosition OPos = transform(Pos);
  FileBuffer_.insert(FileBuffer_.begin() + OPos.Line_, Line);
  changeLineOffsets(Pos, 1);
  return *this;
}

CodeInjector& CodeInjector::insertLineAfter(SourcePosition Pos, const std::string& Line) {
  return insertLineBefore(Pos.changeLine(1), Line);
}

CodeInjector& CodeInjector::insertSubstringBefore(SourcePosition Pos,
                                                  const std::string& Substring) {

  getLine(Pos).insert(transform(Pos).Col_, Substring);

  changeColumnOffsets(Pos, Substring.length());
  return *this;
}

CodeInjector& CodeInjector::insertSubstringAfter(SourcePosition Pos, const std::string& Substring) {
  return insertSubstringBefore(Pos.changeCol(1), Substring);
}

CodeInjector& CodeInjector::eraseLine(SourcePosition Pos) {
  OutputPosition OPos = transform(Pos);
  FileBuffer_.erase(FileBuffer_.begin() + OPos.Line_);
  changeLineOffsets(Pos, -1);
  return *this;
}

CodeInjector& CodeInjector::substituteSubstring(SourcePosition BeginPos, SourcePosition EndPos,
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

  BeginPos.changeLine(1);

  while (BeginPos.Line_ < EndPos.Line_) {
    eraseLine(BeginPos);
    BeginPos.changeLine(1);
  }

  getLine(EndPos).erase(transform({EndPos.Line_, 0}).Col_, transform(EndPos).Col_);

  return *this;
}

size_t CodeInjector::getPositionAsOffset(SourcePosition Pos) {
  size_t Offset = 0;
  for (size_t Line = 0; Line < Pos.Line_; Line++) {
    Offset += ColumnOffsets_[Line].size() + 1;
  }
  Offset += Pos.Col_;
  return Offset;
}

SourcePosition CodeInjector::getOffsetAsPosition(size_t Offset) {
  size_t Line = 0, Col = 0;
  while (Offset >= ColumnOffsets_[Line].size() + 1) {
    Offset -= ColumnOffsets_[Line].size() + 1;
    Line++;
  }
  Col = Offset;
  return {Line, Col};
}

SourcePosition CodeInjector::findFirstEntry(SourcePosition Pos, const std::string& Substring) {

  size_t Offset = getPositionAsOffset(Pos);
  Offset = SourceFile_.find(Substring, Offset);
  assert(Offset != std::string::npos);
  return getOffsetAsPosition(Offset);
}

SourcePosition CodeInjector::findFirstEntry(SourcePosition Pos, char Char) {

  size_t Offset = getPositionAsOffset(Pos);
  Offset = SourceFile_.find_first_of(Char, Offset);
  assert(Offset != std::string::npos);

  return getOffsetAsPosition(Offset);
}

namespace {
const char* TWO_SPEC_SYMBOLS_IN_A_ROW = "The source format contains two SPECIAL symbols in a row";
const char* LAST_SPEC_SYMBOL = "Last symbol in source format is SPECIAL";
const char* NOT_CORRESPONDING_SOURCE_FORMAT =
    "The source format does not correspond to source text";
const char* INCORRECT_ARGUMENT = "The arguments does not correspond to source text";
}; // namespace

IncorrectSubstitutionException::IncorrectSubstitutionException(const char* Message)
    : Message_(Message) {}
const char* IncorrectSubstitutionException::what() const noexcept { return Message_; }

void checkCorrectnessOfFormat(const std::string& SourceFormat) {
  bool isPrevSpecial = false;
  for (const auto& Character : SourceFormat) {
    if (isAnyOf(Character, SPECIAL_SYMBOL::SKIP, SPECIAL_SYMBOL::ANY)) {
      if (isPrevSpecial) {
        throw IncorrectSubstitutionException(TWO_SPEC_SYMBOLS_IN_A_ROW);
      }
      isPrevSpecial = true;
    } else {
      isPrevSpecial = false;
    }
  }
  if (isPrevSpecial)
    throw IncorrectSubstitutionException(LAST_SPEC_SYMBOL);
}

CodeInjector& CodeInjector::substitute(SourcePosition BeginPos, std::string SourceFormat,
                                       std::string OutputFormat,
                                       const std::vector<std::string>& Args) {

  checkCorrectnessOfFormat(SourceFormat);
  SourceFormat += static_cast<char>(SPECIAL_SYMBOL::ARG);
  OutputFormat += static_cast<char>(SPECIAL_SYMBOL::ARG);
  size_t CurArg = 0;

  SourcePosition CurBegin = BeginPos, CurPos = BeginPos;
  size_t CurOutputBegin = 0, CurOutputPos = 0;
  size_t CurFormatPos = 0;

  while (1) {
    while (getChar(CurPos) == SourceFormat[CurFormatPos]) {
      CurPos.changeCol(1), CurFormatPos++;
      if (CurPos.Col_ >= ColumnOffsets_[CurPos.Line_].size()) {
        break;
      }
    }

    if (not isAnySymbol(SourceFormat[CurFormatPos])) {
      throw IncorrectSubstitutionException(NOT_CORRESPONDING_SOURCE_FORMAT);
    }

    // ANY and SKIP

    if (isAnyOf(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ANY, SPECIAL_SYMBOL::SKIP)) {

      SPECIAL_SYMBOL Symb = getAsSymb(SourceFormat[CurFormatPos]);
      CurFormatPos++;

      CurPos = isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG)
                   ? findFirstEntry(CurPos, Args[CurArg])
                   : findFirstEntry(CurPos, SourceFormat[CurFormatPos]);
      if (Symb == SPECIAL_SYMBOL::SKIP) {
        CurBegin = CurPos;
      }
      continue;
    }

    // ARG

    assert(isSymbol(SourceFormat[CurFormatPos], SPECIAL_SYMBOL::ARG));

    while (isNotSymbol(OutputFormat[CurOutputPos], SPECIAL_SYMBOL::ARG)) {
      CurOutputPos++;
    }
    substituteSubstring(CurBegin, CurPos,
                        OutputFormat.substr(CurOutputBegin, CurOutputPos - CurOutputBegin));
    CurOutputBegin = ++CurOutputPos;

    if (CurArg < Args.size()) {
      CurPos = findFirstEntry(CurPos, Args[CurArg]);
      for (const char& ArgChar : Args[CurArg]) {
        if (ArgChar == '\n') {
          CurPos.changeLine(1);
          CurPos.Col_ = 0;
        } else {
          if (getChar(CurPos) != ArgChar) {
            throw IncorrectSubstitutionException(INCORRECT_ARGUMENT);
          }
          CurPos.changeCol(1);
        }
      }
      CurFormatPos++;
      if (isNotAnySymbol(SourceFormat[CurFormatPos])) {
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
