#pragma once

#include <cstddef>
#include <deque>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace ub_tester::code_injector {

using SubstArgs = std::vector<std::string>;

enum class CharacterKind : char { Arg = '@', All = '#', Skip = '$' };

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
  return isAnyOf(C, CharacterKind::Arg, CharacterKind::All,
                 CharacterKind::Skip);
}

enum class SubstPriorityKind : unsigned {
  Shallow /*High?*/ = 0,
  Medium = 1,
  Deep = 2
};

class CodeInjector;
struct Substitution {
  Substitution() = default;
  Substitution(size_t Offset, SubstPriorityKind Prior, std::string SourceFormat,
               std::string OutputFormat, SubstArgs Args)
      : Offset_{Offset}, Prior_{Prior}, SourceFormat_{std::move(SourceFormat)},
        OutputFormat_{std::move(OutputFormat)}, Args_{std::move(Args)} {}

  inline void setOffset(size_t Offset) { Offset_ = Offset; }
  inline void setPrior(SubstPriorityKind Prior) { Prior_ = Prior; }
  inline void setSourceFormat(std::string SourceFormat) {
    SourceFormat_.assign(std::move(SourceFormat));
  }
  inline void setOutputFormat(std::string OutputFormat) {
    OutputFormat_.assign(std::move(OutputFormat));
  }
  inline void setArguments(const SubstArgs& Arguments) { Args_ = Arguments; }

  bool operator<(const Substitution& Other) const;

private:
  friend class CodeInjector;

private:
  size_t Offset_;
  SubstPriorityKind Prior_{SubstPriorityKind::Medium};
  std::string SourceFormat_, OutputFormat_;
  SubstArgs Args_;
};

class CodeInjector {
public:
  CodeInjector() = default;
  CodeInjector(const std::string& InputFilename,
               const std::string& OutputFilename);

  void substitute(Substitution Subst);
  void substitute(size_t Offset, SubstPriorityKind Prior,
                  std::string SourceFormat, std::string OutputFormat,
                  const SubstArgs& Args);

  void applySubstitutions();

  void applySubstitutions(std::istream&, const std::string& OutputFilename);
  void applySubstitutions(const std::string& InputFilename, std::ostream&);
  void applySubstitutions(std::istream&, std::ostream&);

  const std::string& getInputFilename() const;
  const std::string& getOutputFilename() const;

private:
  bool maybeFrontSubstitution(std::istream&, std::ostream&);
  void applyFrontSubstitution(std::istream&, std::ostream&);

private:
  std::optional<std::string> InputFilename_, OutputFilename_;
  std::deque<Substitution> Substitutions_;
};
} // namespace ub_tester::code_injector
