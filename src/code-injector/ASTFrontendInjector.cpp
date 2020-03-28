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
    const SubArgs& Args) {
  std::string Filename = SM.getFilename(Loc).str();
  CodeInjector& Injector = Files_[Filename];
  size_t LineNum = SM.getSpellingLineNumber(Loc);
  size_t BeginPos = SM.getSpellingColumnNumber(Loc);
  Files_[Filename].substitute(
      LineNum, BeginPos, SourceFormat, OutputFormat, Args);
}

} // namespace ub_tester
