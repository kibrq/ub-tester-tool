#include "ASTFrontendInjector.h"

using namespace clang;

namespace ub_tester {

ASTFrontendInjector& ASTFrontendInjector::getInstance() {
  static ASTFrontendInjector Injector;
  return Injector;
}

namespace {
std::string generateOutputFilename(std::string Filename) {
  for (size_t i = Filename.length(); i > 0; i--) {
    if (Filename[i - 1] == '/') {
      Filename.insert(i, "IMPROVED_");
      break;
    }
  }
  return Filename;
}

size_t getLine(const SourceManager& SM, const SourceLocation& Loc) {
  return SM.getSpellingLineNumber(Loc) - 1;
}

size_t getCol(const SourceManager& SM, const SourceLocation& Loc) {
  return SM.getSpellingColumnNumber(Loc) - 1;
}
} // namespace

void ASTFrontendInjector::addFile(const SourceManager& SM) {
  std::string Filename =
      SM.getFilename(SM.getLocForStartOfFile(SM.getMainFileID())).str();
  Files_.emplace(
      Filename, Filename); // Constructs CodeInjectr from Filename in-place
  Files_[Filename].setOutputFilename(generateOutputFilename(Filename));
  llvm::outs() << generateOutputFilename(Filename) << '\n';
}

void ASTFrontendInjector::insertLineBefore(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& Line) {
  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = getLine(SM, Loc);
  Files_[Filename].insertLineBefore(LineNum, Line);
}

void ASTFrontendInjector::insertLineAfter(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& Line) {
  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = getLine(SM, Loc);
  Files_[Filename].insertLineAfter(LineNum, Line);
}

void ASTFrontendInjector::substitute(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& SourceFormat, const std::string& OutputFormat,
    const SubArgs& Args) {
  std::string Filename = SM.getFilename(Loc).str();
  CodeInjector& Injector = Files_[Filename];
  size_t LineNum = getLine(SM, Loc);
  size_t BeginPos = getCol(SM, Loc);
  Files_[Filename].substitute(
      LineNum, BeginPos, SourceFormat, OutputFormat, Args);
}

void ASTFrontendInjector::substituteSubstring(
    const SourceManager& SM, const SourceLocation& Begin,
    const SourceLocation& End, const std::string& Substitution) {
  std::string Filename = SM.getFilename(Begin).str();
  size_t LineNumBegin = getLine(SM, Begin);
  size_t LineNumEnd = getLine(SM, End);
  size_t ColBegin = getCol(SM, Begin);
  size_t ColEnd = getCol(SM, End);
  Files_[Filename].substituteSubstring(
      LineNumBegin, ColBegin, LineNumEnd, ColEnd, Substitution);
}

void ASTFrontendInjector::substituteSubstring(
    const SourceManager& SM, const SourceRange& Range,
    const std::string& Substitution) {
  substituteSubstring(SM, Range.getBegin(), Range.getEnd(), Substitution);
}

void ASTFrontendInjector::substituteSubstring(
    const SourceManager& SM, const CharSourceRange& Range,
    const std::string& Substitution) {
  substituteSubstring(SM, Range.getBegin(), Range.getEnd(), Substitution);
}

} // namespace ub_tester
