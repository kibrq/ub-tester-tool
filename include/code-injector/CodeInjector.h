#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ub_tester {

namespace code_injector {

using SubArgs = std::vector<std::string>;

enum class CharacterKind : char { ARG = '@', ALL = '#', NONE = 0 };

inline bool isCharacter(char C, CharacterKind Char) {
  return C == static_cast<char>(Char);
}

inline bool isAnyOf(char C, CharacterKind Char1, CharacterKind Char2) {
  return isCharacter(C, Char1) || isCharacter(C, Char2);
}

template <typename... CharacterKinds>
bool isAnyOf(char C, CharacterKind Char1, CharacterKind Char2,
             CharacterKinds... Chars) {
  return isCharacter(C, Char1) || isAnyOf(C, Char2, Chars...);
}

inline bool isAnyCharacter(char C) {
  return isAnyOf(C, CharacterKind::ARG, CharacterKind::ALL);
}

class CodeInjector {
public:
  CodeInjector(const std::string& Filename);
  CodeInjector(const std::string& Filename, const std::string& OutputFilename);
  ~CodeInjector();
  void setOutputFilename(const std::string& Filename);

  void substitute(size_t LineNum, size_t ColNum, std::string SourceFormat,
                  std::string OutputFormat, const SubArgs& Args);

  void substitute(size_t Offset, std::string SourceFormat,
                  std::string OutputFormat, const SubArgs& Args);

private:
  void erase(size_t Offeset, size_t Count);
  void eraseAfter(size_t Offeset, size_t Count);
  void insert(size_t Offset, std::string_view NewString);
  void insertAfter(size_t Offset, std::string_view NewString);
  void substitute(size_t Offset, size_t Count, std::string_view NewString);
  void substituteAfter(size_t Offset, size_t Count, std::string_view NewString);

private:
  inline static constexpr int INVALID = -1e5;

private:
  struct Substitution {
    bool operator<(const Substitution& Other) const;
    size_t Offset_;
    std::string SourceFormat_;
    std::string OutputFormat_;
    SubArgs Args_;
  };
  void applySubstitution(const Substitution& Sub);
  void applySubstitution(size_t Offset, std::string_view SourceFormat,
                         std::string_view OutputFormat, const SubArgs& Args);
  void applyAllSubstitutions();

private:
  std::optional<size_t> findFirstEntryOf(size_t Offset, std::string_view View);
  std::optional<size_t> findFirstEntryOf(size_t Offset, char C);
  char get(size_t Offset) const;
  void updateOffsets(size_t Offset, size_t Value);
  void makeInvalid(size_t Offset, size_t Count);
  bool isValid(size_t Offset) const;
  size_t findFirstValid(size_t Offset) const;
  size_t findFirstValidNextTransformed(size_t Offset) const;
  size_t transform(size_t SourceOffset) const;

private:
  std::vector<char> FileBuffer_;
  std::string SourceBuffer_;
  std::vector<int> Offsets_;
  std::string InputFilename_;
  std::string OutputFilename_;
  std::vector<Substitution> Substitutions_;
};
} // namespace code_injector
} // namespace ub_tester
