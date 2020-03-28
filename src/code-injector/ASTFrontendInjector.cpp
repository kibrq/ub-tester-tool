#include "ASTFrontendInjector.h"

using namespace clang;

namespace ub_tester {

ASTFrontendInjector& ASTFrontendInjector::getInstance() {
  static ASTFrontendInjector Injector;
  return Injector;
}

void ASTFrontendInjector::addFile(const SourceManager& SM) {
  std::string Filename =
      SM.getFilename(SM.getLocForStartOfFile(SM.getMainFileID())).str();
  CodeInjector Injector(Filename);
  Files_.emplace(
      Filename, Filename); // Constructs CodeInjectr from Filename in-place
}

void ASTFrontendInjector::insertLineBefore(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& Line) {
  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = SM.getSpellingLineNumber(Loc);
  Files_[Filename].insertLineBefore(LineNum, Line);
}

void ASTFrontendInjector::insertLineAfter(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& Line) {
  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = SM.getSpellingLineNumber(Loc);
  Files_[Filename].insertLineAfter(LineNum, Line);
}

void ASTFrontendInjector::substitute(
    const SourceManager& SM, const SourceLocation& Loc,
    const std::string& SourceFormat, const std::string& OutputFormat,
    const std::vector<std::string>& Args) {
  std::string Filename = SM.getFilename(Loc).str();
  CodeInjector& Injector = Files_[Filename];
  size_t LineNum = SM.getSpellingLineNumber(Loc);
  size_t BeginPos = SM.getSpellingColumnNumber(Loc);
  Files_[Filename].substitute(
      LineNum, BeginPos, SourceFormat, OutputFormat, Args);
}

} // namespace ub_tester
