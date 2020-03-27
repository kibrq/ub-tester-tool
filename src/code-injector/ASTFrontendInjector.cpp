#include "ASTFrontendInjector.h"

using namespace clang;

namespace ub_tester {

void ASTFrontendInjector::addFile(const SourceManager* SM) {
  SM_ = SM;
  std::string Filename =
      SM_->getFilename(SM_->getLocForStartOfFile(SM_->getMainFileID())).str();
  CodeInjector Injector(Filename);
  Files_.emplace(
      Filename, Filename); // Constructs CodeInjectr from Filename in-place
}

void ASTFrontendInjector::insertLineBefore(
    const SourceLocation& Loc, const std::string& Line) {
  std::string Filename = SM_->getFilename(Loc).str();
  size_t LineNum = SM_->getSpellingLineNumber(Loc);
  Files_[Filename].insertLineBefore(LineNum, Line);
}

void ASTFrontendInjector::insertLineAfter(
    const SourceLocation& Loc, const std::string& Line) {
  std::string Filename = SM_->getFilename(Loc).str();
  size_t LineNum = SM_->getSpellingLineNumber(Loc);
  Files_[Filename].insertLineAfter(LineNum, Line);
}

void ASTFrontendInjector::substitute(
    const SourceLocation& Loc, const std::string& SourceFormat,
    const std::string& OutputFormat, const std::vector<std::string> Args) {
  std::string Filename = SM_->getFilename(Loc).str();
  CodeInjector& Injector = Files_[Filename];
  size_t LineNum = SM_->getSpellingLineNumber(Loc);
  size_t BeginPos = SM_->getSpellingColumnNumber(Loc);
  Files_[Filename].substitute(
      LineNum, BeginPos, SourceFormat, OutputFormat, Args);
}

} // namespace ub_tester
