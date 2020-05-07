#include "code-injector/CodeInjector.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>

namespace ub_tester {
namespace code_injector {

CodeInjector::CodeInjector(const std::string& InputFilename,
                           const std::string& OutputFilename)
    : InputFilename_{InputFilename}, OutputFilename_{OutputFilename} {}

void CodeInjector::applySubstitutions() {
  std::ifstream IStream(InputFilename_, std::ios::in);
  while (IStream.peek() != EOF) {
    FileBuffer_.emplace_back(IStream.get());
  }
  Offsets_.resize(FileBuffer_.size());
  SourceBuffer_.resize(FileBuffer_.size());
  std::copy(FileBuffer_.begin(), FileBuffer_.end(), SourceBuffer_.begin());
  std::stable_sort(Substitutions_.begin(), Substitutions_.end());
  for (const Substitution& Sub : Substitutions_) {
    applySubstitution(Sub);
  }
  IStream.close();
  std::ofstream OStream(OutputFilename_, std::ios::out);
  std::copy(FileBuffer_.begin(), FileBuffer_.end(),
            std::ostream_iterator<char>(OStream));
}

void CodeInjector::updateOffsets(size_t Offset, size_t Value) {
  std::for_each(Offsets_.begin() + Offset, Offsets_.end(), [Value](auto& Off) {
    if (Off != INVALID)
      Off += Value;
  });
}

void CodeInjector::makeInvalid(size_t Offset, size_t Count) {
  std::for_each(Offsets_.begin() + Offset, Offsets_.begin() + Offset + Count,
                [](auto& Off) { Off = INVALID; });
}

bool CodeInjector::isValid(size_t Offset) const {
  return Offsets_[Offset] != INVALID;
}

size_t CodeInjector::transform(size_t SourceOffset) const {
  return SourceOffset + Offsets_[SourceOffset];
}

char CodeInjector::get(size_t Offset) const {
  return FileBuffer_[transform(Offset)];
}

size_t CodeInjector::findFirstValid(size_t Offset) const {
  while (Offset - 1 > 0 && !isValid(Offset))
    --Offset;
  return isValid(Offset) ? Offset : Offset - 1;
}

size_t CodeInjector::findFirstValidNextTransformed(size_t Offset) const {
  size_t NewOffset = findFirstValid(Offset);
  return transform(NewOffset) + (NewOffset != Offset);
}

void CodeInjector::erase(size_t Offset, size_t Count) {
  if (Count == 0)
    return;
  size_t CurOffset = findFirstValidNextTransformed(Offset);
  FileBuffer_.erase(FileBuffer_.begin() + CurOffset,
                    FileBuffer_.begin() + CurOffset + Count);
  makeInvalid(Offset, Count);
  updateOffsets(Offset + Count, -Count);
}

void CodeInjector::eraseAfter(size_t Offset, size_t Count) {
  if (Count == 0)
    return;

  size_t CurOffset = transform(Offset) + 1;
  FileBuffer_.erase(FileBuffer_.begin() + CurOffset,
                    FileBuffer_.begin() + CurOffset + Count);
  makeInvalid(Offset + 1, Count);
  updateOffsets(Offset + 1 + Count, -Count);
}

void CodeInjector::insert(size_t Offset, std::string_view NewString) {
  size_t CurOffset = findFirstValidNextTransformed(Offset);
  FileBuffer_.insert(FileBuffer_.begin() + CurOffset, NewString.begin(),
                     NewString.end());
  updateOffsets(Offset, NewString.length());
}

void CodeInjector::insertAfter(size_t Offset, std::string_view NewString) {
  size_t CurOffset = transform(Offset) + 1;
  FileBuffer_.insert(FileBuffer_.begin() + CurOffset, NewString.begin(),
                     NewString.end());
  updateOffsets(Offset + 1, NewString.length());
}

void CodeInjector::substitute(size_t Offset, size_t Count,
                              std::string_view NewString) {
  erase(Offset, Count);
  insert(Offset, NewString);
}

void CodeInjector::substituteAfter(size_t Offset, size_t Count,
                                   std::string_view NewString) {
  eraseAfter(Offset, Count);
  insertAfter(Offset, NewString);
}

std::optional<size_t> CodeInjector::findFirstEntryOf(size_t Offset,
                                                     std::string_view View) {
  size_t Result = SourceBuffer_.find(View, Offset);
  if (Result != std::string::npos) {
    return Result;
  }
  return {};
}

std::optional<size_t> CodeInjector::findFirstEntryOf(size_t Offset, char C) {
  size_t Result = SourceBuffer_.find_first_of(C, Offset);
  if (Result != std::string::npos) {
    return Result;
  }
  return {};
}

void CodeInjector::substitute(size_t LineNum, size_t ColNum,
                              std::string SourceFormat,
                              std::string OutputFormat, const SubArgs& Args) {
  size_t Offset = 0;
  for (size_t i = 0; i < LineNum - 1; ++i) {
    if (auto Ans = findFirstEntryOf(Offset, '\n'); Ans.has_value()) {
      Offset = *Ans + 1;

    } else {
      // TODO
    }
  }
  substitute(Offset + ColNum - 1, std::move(SourceFormat),
             std::move(OutputFormat), Args);
}

bool CodeInjector::Substitution::operator<(const Substitution& Other) const {

  if (Offset_ == Other.Offset_) {
    size_t Len1 = 0, Len2 = 0;
    for (const auto& A : Args_) {
      Len1 += A.length();
    }
    for (const auto& A : Other.Args_) {
      Len2 += A.length();
    }
    return Len1 > Len2;
  }
  return Offset_ < Other.Offset_;
}

void CodeInjector::substitute(size_t Offset, std::string SourceFormat,
                              std::string OutputFormat, const SubArgs& Args) {
  Substitutions_.push_back(
      {Offset, std::move(SourceFormat), std::move(OutputFormat), Args});
}

void CodeInjector::applySubstitution(const Substitution& Sub) {
  applySubstitution(Sub.Offset_, Sub.SourceFormat_, Sub.OutputFormat_,
                    Sub.Args_);
}

void CodeInjector::applySubstitution(size_t Offset,
                                     std::string_view SourceFormat,
                                     std::string_view OutputFormat,
                                     const SubArgs& Args) {
  size_t CurSourceBegin = Offset, CurSourcePos = Offset;
  size_t CurArg = 0;
  bool isPrevAny = false;
  for (const auto& C : SourceFormat) {
    if (isAnyCharacter(C)) {
      switch (static_cast<CharacterKind>(C)) {
      case CharacterKind::ARG: {
        if (auto Ans = findFirstEntryOf(CurSourcePos, Args[CurArg]);
            Ans.has_value()) {
          CurSourcePos = *Ans;
        } else {
          // TODO
        }
        size_t Res = OutputFormat.find_first_of(C, 0);
        substitute(CurSourceBegin, CurSourcePos - CurSourceBegin,
                   OutputFormat.substr(0, Res));
        OutputFormat.remove_prefix(Res + 1);
        CurSourcePos += Args[CurArg++].length();
        CurSourceBegin = CurSourcePos;
        break;
      }
      case CharacterKind::ALL: {
        isPrevAny = true;
        continue;
      }
      case CharacterKind::NONE:
        std::abort();
      }
    } else {
      if (isPrevAny) {
        if (auto Ans = findFirstEntryOf(CurSourcePos, C); Ans.has_value()) {
          CurSourcePos = *Ans + 1;

        } else {
          // TODO
        }
      } else {
        if (get(CurSourcePos++) != C) {
          // TODO
        }
      }
    }
    isPrevAny = false;
  }
  if (SourceFormat.size() == 0) {
    insert(Offset, OutputFormat);
  } else {
    substituteAfter(CurSourceBegin - 1, CurSourcePos - CurSourceBegin,
                    OutputFormat);
  }
}
} // namespace code_injector
} // namespace ub_tester
