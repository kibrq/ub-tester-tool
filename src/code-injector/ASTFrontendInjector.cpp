#include "clang/Basic/SourceManager.h"

#include "code-injector/ASTFrontendInjector.h"

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

void ASTFrontendInjector::addFile(const std::string& Filename) {
  Files_.emplace(Filename,
                 Filename); // Constructs CodeInjectr from Filename in-place
  Files_[Filename].setOutputFilename(generateOutputFilename(Filename));
}

void ASTFrontendInjector::addFile(const ASTContext* Context) {
  const SourceManager& SM = Context->getSourceManager();
  std::string Filename =
      SM.getFilename(SM.getLocForStartOfFile(SM.getMainFileID())).str();
  addFile(Filename);
  llvm::outs() << generateOutputFilename(Filename) << '\n';
}

void ASTFrontendInjector::insertLineBefore(const ASTContext* Context,
                                           const SourceLocation& Loc,
                                           const std::string& Line) {
  const SourceManager& SM = Context->getSourceManager();

  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = getLine(SM, Loc);
  Files_[Filename].insertLineBefore({LineNum, 0}, Line);
}

void ASTFrontendInjector::insertLineAfter(const ASTContext* Context,
                                          const SourceLocation& Loc,
                                          const std::string& Line) {
  const SourceManager& SM = Context->getSourceManager();

  std::string Filename = SM.getFilename(Loc).str();
  size_t LineNum = getLine(SM, Loc);
  Files_[Filename].insertLineAfter({LineNum, 0}, Line);
}

void ASTFrontendInjector::substitute(const ASTContext* Context,
                                     const SourceLocation& Loc,
                                     const std::string& SourceFormat,
                                     const std::string& OutputFormat,
                                     const SubArgs& Args) {
  const SourceManager& SM = Context->getSourceManager();

  std::string Filename = SM.getFilename(Loc).str();
  CodeInjector& Injector = Files_[Filename];
  size_t LineNum = getLine(SM, Loc);
  size_t BeginPos = getCol(SM, Loc);
  Files_[Filename].substitute({LineNum, BeginPos}, SourceFormat, OutputFormat,
                              Args);
}

void ASTFrontendInjector::substituteSubstring(const ASTContext* Context,
                                              const SourceLocation& Begin,
                                              const SourceLocation& End,
                                              const std::string& Substitution) {
  const SourceManager& SM = Context->getSourceManager();

  std::string Filename = SM.getFilename(Begin).str();
  size_t LineNumBegin = getLine(SM, Begin);
  size_t LineNumEnd = getLine(SM, End);
  size_t ColBegin = getCol(SM, Begin);
  size_t ColEnd = getCol(SM, End);
  Files_[Filename].substituteSubstring({LineNumBegin, ColBegin},
                                       {LineNumEnd, ColEnd}, Substitution);
}

void ASTFrontendInjector::substituteSubstring(const ASTContext* Context,
                                              const SourceRange& Range,
                                              const std::string& Substitution) {

  substituteSubstring(Context, Range.getBegin(), Range.getEnd(), Substitution);
}

void ASTFrontendInjector::substituteSubstring(const ASTContext* Context,
                                              const CharSourceRange& Range,
                                              const std::string& Substitution) {

  substituteSubstring(Context, Range.getBegin(), Range.getEnd(), Substitution);
}

} // namespace ub_tester
