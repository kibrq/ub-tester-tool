#include "clang/Basic/SourceManager.h"

#include "code-injector/ASTFrontendInjector.h"

using namespace clang;
using namespace ub_tester::code_injector;

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
  return SM.getSpellingLineNumber(Loc);
}

size_t getCol(const SourceManager& SM, const SourceLocation& Loc) {
  return SM.getSpellingColumnNumber(Loc);
}
} // namespace

void ASTFrontendInjector::addFile(const std::string& Filename) {
  Injector = std::make_unique<CodeInjector>(Filename,
                                            generateOutputFilename(Filename));
}

void ASTFrontendInjector::addFile(const ASTContext* Context) {
  const SourceManager& SM = Context->getSourceManager();
  addFile(SM.getFileEntryForID(SM.getMainFileID())->getName().str());
}

void ASTFrontendInjector::substitute(const ASTContext* Context,
                                     const SourceLocation& Loc,
                                     std::string SourceFormat,
                                     std::string OutputFormat,
                                     const SubArgs& Args) {
  const SourceManager& SM = Context->getSourceManager();
  Injector->substitute(SM.getFileOffset(Loc), std::move(SourceFormat),
                       std::move(OutputFormat), Args);
}

} // namespace ub_tester
