#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ub_tester {

namespace code_injector {

using SubArgs = std::vector<std::string>;

enum class CharacterKind : char { ARG = '@', ALL = '#', NONE = 0 };

inline bool isCharacter(char C, CharacterKind Char) { return C == static_cast<char>(Char); }

inline bool isAnyOf(char C, CharacterKind Char1, CharacterKind Char2) {
  return isCharacter(C, Char1) || isCharacter(C, Char2);
}

template <typename... CharacterKinds>
bool isAnyOf(char C, CharacterKind Char1, CharacterKind Char2, CharacterKinds... Chars) {
  return isCharacter(C, Char1) || isAnyOf(C, Char2, Chars...);
}

inline bool isAnyCharacter(char C) { return isAnyOf(C, CharacterKind::ARG, CharacterKind::ALL); }

class CodeInjector {
public:
  CodeInjector(const std::string& InputFilename, const std::string& OutputFilename);

  void substitute(
      size_t Offset, std::string SourceFormat, std::string OutputFormat, const SubArgs& Args);

  void applySubstitutions();

private:
  bool maybeFrontSubstitution(std::istream&, std::ostream&);
  void applyFrontSubstitution(std::istream&, std::ostream&);

private:
  struct Substitution {
    Substitution(size_t Offset, std::string SourceFormat, std::string OutputFormat, SubArgs Args)
        : Offset_{Offset},
          SourceFormat_{std::move(SourceFormat)},
          OutputFormat_{std::move(OutputFormat)},
          Args_{std::move(Args)} {}

    bool operator<(const Substitution& Other) const;
    size_t Offset_;
    std::string SourceFormat_;
    std::string OutputFormat_;
    SubArgs Args_;
  };

private:
  std::string InputFilename_, OutputFilename_;
  std::deque<Substitution> Substitutions_;
};
} // namespace code_injector
} // namespace ub_tester
