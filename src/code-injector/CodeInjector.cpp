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
  std::ifstream IStream(*InputFilename_, std::ios::in | std::ios::binary);
  std::ofstream OStream(*OutputFilename_, std::ios::out | std::ios::binary);
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
  while (IStream.peek() != EOF) {
    if (!maybeFrontSubstitution(IStream, OStream))
      OStream << static_cast<char>(IStream.get());
  }
}

const std::string& CodeInjector::getInputFilename() const {
  assert(InputFilename_);
  return *InputFilename_;
}

const std::string& CodeInjector::getOutputFilename() const {
  assert(OutputFilename_);
  return *OutputFilename_;
}

bool Substitution::operator<(const Substitution& Other) const {
  if (Offset_ != Other.Offset_) {
    return Offset_ < Other.Offset_;
  }
  if (Prior_ != Other.Prior_) {
    return static_cast<unsigned>(Prior_) < static_cast<unsigned>(Other.Prior_);
  }
  size_t Len1 = 0, Len2 = 0;
  for (const auto& A : Args_) {
    Len1 += A.length();
  }
  for (const auto& A : Other.Args_) {
    Len2 += A.length();
  }
  return Len1 > Len2;
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
  std::vector<unsigned> NeedleOffsets(
      std::numeric_limits<unsigned char>::max());
  for (auto& Off : NeedleOffsets) {
    Off = Needle.length();
  }
  for (unsigned I = 0; I < Needle.length() - 1; ++I) {
    NeedleOffsets[Needle[I]] = Needle.length() - I - 1;
  }
  IStream.seekg(Needle.length() - 1, std::ios_base::cur);
  auto MoveBackward = [](std::istream& Stream) {
    if (Stream.tellg() > 0)
      Stream.seekg(-1, std::ios_base::cur);
  };
  while (IStream.peek() != EOF) {
    bool Flag = true;
    for (unsigned I = 0; I < Needle.length(); ++I, MoveBackward(IStream)) {
      if (IStream.peek() != Needle[Needle.length() - I - 1]) {
        Flag = false;
        break;
      }
    }
    if (Flag) {
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
  if (isCharacter(Char, CharacterKind::Arg)) {
    findFirstEntryOf(IStream, NextArg);
  } else if (!isAnyCharacter(Char)) {
    findFirstEntryOf(IStream, Char);
  }
}

} // namespace

bool CodeInjector::maybeFrontSubstitution(std::istream& IStream,
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
  unsigned CurArg = 0;
  bool isPrevSkip = false;
  int PrevPos = 0;
  for (const auto& Char : Sub.SourceFormat_) {
    findNextCharacter(IStream, Char, Sub.Args_[CurArg]);
    if (isPrevSkip) {
      isPrevSkip = false;
      int EndPos = IStream.tellg();
      IStream.seekg(PrevPos - EndPos, std::ios_base::cur);
      for (; PrevPos < EndPos; ++PrevPos) {
        OStream << static_cast<char>(IStream.get());
      }
    }
    switch (Char) {
    case static_cast<char>(CharacterKind::Arg): {
      unsigned Pos = OutputFormat.find_first_of(Char);
      std::copy_n(OutputFormat.begin(), Pos,
                  std::ostream_iterator<char>(OStream));
      unsigned StartPos = IStream.tellg();
      while (IStream.tellg() < StartPos + Sub.Args_[CurArg].length())
        if (!maybeFrontSubstitution(IStream, OStream)) {
          OStream << static_cast<char>(IStream.get());
        }
      OutputFormat.remove_prefix(Pos + 1), ++CurArg;
      break;
    }
    case static_cast<char>(CharacterKind::Skip): {
      isPrevSkip = true;
      PrevPos = IStream.tellg();
      break;
    }
    case static_cast<char>(CharacterKind::All): {
    default:
      break;
    }
    }
  }
  std::copy(OutputFormat.begin(), OutputFormat.end(),
            std::ostream_iterator<char>(OStream));
}
} // namespace ub_tester::code_injector
