#include "code-injector/ASTFrontendInjector.h"
#include "UBUtility.h"

#include "clang/Basic/SourceManager.h"

#include <experimental/filesystem>
#include <fstream>
#include <string_view>
#include <unordered_set>

using namespace clang;
using namespace ub_tester::code_injector;
namespace fs = std::experimental::filesystem;

namespace ub_tester {

std::unique_ptr<ASTFrontendInjector> ASTFrontendInjector::Instance_{nullptr};

void ASTFrontendInjector::initialize(
    const std::vector<std::string>& SourcePathList) {
  static bool Inited = false;
  assert(!Inited);
  Inited = true;
  for (const auto& SourcePath : SourcePathList) {
    getInstance().addFile(fs::absolute(SourcePath));
  }
}

ASTFrontendInjector& ASTFrontendInjector::getInstance() {
  if (!Instance_) {
    Instance_.reset(new ASTFrontendInjector());
  }
  return *Instance_;
}

namespace {
inline constexpr char UBTesterPrefix[] = "UBTested_";
inline constexpr char TargetKeyword[] = "#include";

std::string generateOutputFilename(std::string Filename) {
  fs::path Path{std::move(Filename)};
  Path.replace_filename(UBTesterPrefix +
                        static_cast<std::string>(Path.filename()));
  return static_cast<std::string>(Path);
}
} // namespace

void ASTFrontendInjector::substituteIncludePaths() {
  std::unordered_set<std::string> AvailFilenames;
  auto& Files = getInstance().Injectors_;
  for (const auto& [Filename, Inj] : Files) {
    AvailFilenames.insert(
        static_cast<std::string>(fs::path{Filename}.filename()));
  }
  for (const auto& [Filename, Inj] : Files) {
    std::ifstream IStream(Filename, std::ios::in);
    std::string Word;
    bool isIncludePrev = false;
    while (IStream >> Word) {
      if (Word.compare(TargetKeyword) == 0) {
        isIncludePrev = true;
        continue;
      }
      if (!isIncludePrev) {
        continue;
      }
      // Don't really know why Word.length() - 2...
      if (std::string IncludeFilename = static_cast<std::string>(
              fs::path(Word.substr(1, Word.length() - 2)).filename());
          AvailFilenames.find(IncludeFilename) != AvailFilenames.end()) {
        unsigned Pos = IStream.tellg();
        Inj->substitute(Pos - Word.length(), "$@",
                        UBTesterPrefix + std::string{"@"}, {IncludeFilename});
      }
      isIncludePrev = false;
    }
  }
}

void ASTFrontendInjector::addFile(const std::string& Filename) {
  Injectors_.emplace(Filename, std::make_unique<CodeInjector>(
                                   Filename, generateOutputFilename(Filename)));
}

void ASTFrontendInjector::applySubstitutions() {
  for (auto& [Filename, Inj] : Injectors_) {
    Inj->applySubstitutions();
  }
}

void ASTFrontendInjector::substitute(const ASTContext* Context,
                                     const SourceLocation& Loc,
                                     std::string SourceFormat,
                                     std::string OutputFormat,
                                     const SubArgs& Args) {
  const SourceManager& SM = Context->getSourceManager();
  Injectors_[SM.getFilename(Loc).str()]->substitute(
      SM.getFileOffset(Loc), std::move(SourceFormat), std::move(OutputFormat),
      Args);
}

void ASTFrontendInjector::substitute(const clang::ASTContext* Context,
                                     const clang::SourceRange& Range,
                                     std::string NewString) {
  substitute(Context, Range.getBegin(), getRangeAsString(Range, Context),
             NewString, SubArgs({}));
}

} // namespace ub_tester
