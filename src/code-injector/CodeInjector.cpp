#include "code-injector/CodeInjector.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <limits>
#include <string_view>

namespace ub_tester::code_injector {

CodeInjector::CodeInjector(const std::string& InputFilename,
                           const std::string& OutputFilename)
    : InputFilename_{InputFilename}, OutputFilename_{OutputFilename} {}

void CodeInjector::applySubstitutions() {
  assert(InputFilename_.has_value() && OutputFilename_.has_value());
  std::ifstream IStream(InputFilename_.value(),
                        std::ios::in | std::ios::binary);
  std::ofstream OStream(OutputFilename_.value(),
                        std::ios::out | std::ios::binary);
  applySubstitutions(IStream, OStream);
}

void CodeInjector::applySubstitutions(std::istream& IStream,
                                      const std::string& OutputFilename) {
  std::ofstream OStream(OutputFilename, std::ios::out | std::ios::binary);
  applySubstitutions(IStream, OStream);
}

void CodeInjector::applySubstitutions(const std::string& InputFilename,
                                      std::ostream& OStream) {
  std::ifstream IStream(InputFilename, std::ios::in | std::ios::binary);
  applySubstitutions(IStream, OStream);
}

void CodeInjector::applySubstitutions(std::istream& IStream,
                                      std::ostream& OStream) {
  std::sort(Substitutions_.begin(), Substitutions_.end());
  Substitutions_.erase(
      std::unique(Substitutions_.begin(), Substitutions_.end()),
      Substitutions_.end());

  while (IStream.peek() != EOF)
    if (!applyFrontSubstitutionIfNeed(IStream, OStream))
      OStream << static_cast<char>(IStream.get());
}

const std::string& CodeInjector::getInputFilename() const {
  assert(InputFilename_.has_value());
  return InputFilename_.value();
}

const std::string& CodeInjector::getOutputFilename() const {
  assert(OutputFilename_.has_value());
  return OutputFilename_.value();
}

bool Substitution::operator<(const Substitution& Other) const {
  if (Offset_ != Other.Offset_)
    return Offset_ < Other.Offset_;
  if (Prior_ != Other.Prior_)
    return static_cast<int>(Prior_) < static_cast<int>(Other.Prior_);

  size_t Len1 = 0, Len2 = 0;
  for (const auto& Arg : Args_)
    Len1 += Arg.length();
  for (const auto& Arg : Other.Args_)
    Len2 += Arg.length();
  return Len1 > Len2;
}

bool Substitution::operator==(const Substitution& Other) const {
  if (Offset_ != Other.Offset_)
    return false;
  if (SourceFormat_.compare(Other.SourceFormat_) != 0)
    return false;
  if (Args_ != Other.Args_)
    return false;
  return true;
}

void CodeInjector::substitute(Substitution Subst) {
  Substitutions_.emplace_back(std::move(Subst));
}

void CodeInjector::substitute(size_t Offset, SubstPriorityKind Prior,
                              std::string SourceFormat,
                              std::string OutputFormat, const SubstArgs& Args) {
  Substitutions_.emplace_back(Offset, Prior, std::move(SourceFormat),
                              std::move(OutputFormat), Args);
}

namespace {

void findFirstEntryOf(std::istream& IStream, std::string_view Needle) {
  std::vector<size_t> NeedleOffsets(std::numeric_limits<unsigned char>::max());
  for (auto& Offset : NeedleOffsets)
    Offset = Needle.length();
  for (size_t I = 0, Size = Needle.length(); I < Size - 1; ++I)
    NeedleOffsets[Needle[I]] = Size - I - 1;

  IStream.seekg(Needle.length() - 1, std::ios_base::cur);
  auto MoveBackward = [](std::istream& Stream) {
    if (Stream.tellg() > 0)
      Stream.seekg(-1, std::ios_base::cur);
  };
  while (IStream.peek() != EOF) {
    bool MatchFound = true;
    for (size_t I = 0, Size = Needle.length(); I < Size;
         ++I, MoveBackward(IStream))
      if (IStream.peek() != Needle[Size - I - 1]) {
        MatchFound = false;
        break;
      }
    if (MatchFound) {
      if (IStream.tellg() > 0)
        IStream.get();
      return;
    } else {
      IStream.seekg(NeedleOffsets[IStream.peek()], std::ios_base::cur);
    }
  }
}

void findFirstEntryOf(std::istream& IStream, char Char) {
  while (IStream.peek() != EOF && IStream.get() != Char)
    ;
  return;
}

void findNextCharacter(std::istream& IStream, char Char,
                       const std::string& NextArg) {
  if (isCharacterKind(Char, CharacterKind::Arg))
    findFirstEntryOf(IStream, NextArg);
  else if (!isAnyCharacter(Char))
    findFirstEntryOf(IStream, Char);
}

} // namespace

bool CodeInjector::applyFrontSubstitutionIfNeed(std::istream& IStream,
                                                std::ostream& OStream) {
  if (!Substitutions_.empty() &&
      Substitutions_.front().Offset_ == IStream.tellg()) {
    applyFrontSubstitution(IStream, OStream);
    return true;
  }
  assert(Substitutions_.empty() ||
         Substitutions_.front().Offset_ > IStream.tellg());
  return false;
}

void CodeInjector::applyFrontSubstitution(std::istream& IStream,
                                          std::ostream& OStream) {
  Substitution Sub = std::move(Substitutions_.front());
  Substitutions_.pop_front();
  std::string_view OutputFormat = Sub.OutputFormat_;
  size_t CurArg = 0;
  bool IsPrevSkip = false;
  int PrevPos = 0;
  for (const auto& Symb : Sub.SourceFormat_) {
    findNextCharacter(IStream, Symb, Sub.Args_[CurArg]);
    if (IsPrevSkip) {
      IsPrevSkip = false;
      int EndPos = IStream.tellg();
      IStream.seekg(PrevPos - EndPos, std::ios_base::cur);
      for (; PrevPos < EndPos; ++PrevPos)
        OStream << static_cast<char>(IStream.get());
    }
    switch (Symb) {
    case static_cast<char>(CharacterKind::Arg): {
      size_t Pos = OutputFormat.find_first_of(Symb);
      std::copy_n(OutputFormat.begin(), Pos,
                  std::ostream_iterator<char>(OStream));
      size_t StartPos = IStream.tellg();
      while (IStream.tellg() < StartPos + Sub.Args_[CurArg].length())
        if (!applyFrontSubstitutionIfNeed(IStream, OStream))
          OStream << static_cast<char>(IStream.get());
      OutputFormat.remove_prefix(Pos + 1), ++CurArg;
      break;
    }
    case static_cast<char>(CharacterKind::Skip): {
      IsPrevSkip = true;
      PrevPos = IStream.tellg();
      break;
    }
    default:
      break;
    }
  }
  std::copy(OutputFormat.begin(), OutputFormat.end(),
            std::ostream_iterator<char>(OStream));
}

} // namespace ub_tester::code_injector
