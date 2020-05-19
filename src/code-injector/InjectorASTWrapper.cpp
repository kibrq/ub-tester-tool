#include "code-injector/InjectorASTWrapper.h"
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

InjectorASTWrapper& InjectorASTWrapper::getInstance() {
  static InjectorASTWrapper Injector;
  return Injector;
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

void InjectorASTWrapper::substituteIncludePaths() {
  std::unordered_set<std::string> AvailFilenames;
  auto& Files = getInstance().Injectors_;
  /*for (const auto& [Filename, Inj] : Files) {
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
         Inj->substitute(Pos - Word.length(), SubstPriorityKind::Medium, "$@",
                         UBTesterPrefix + std::string{"@"}, {IncludeFilename});
       }
       isIncludePrev = false;
     }
   }
   */
}

void InjectorASTWrapper::addFile(const ASTContext* Context) {
  const auto& SM = Context->getSourceManager();
  auto ID = SM.getMainFileID();
  std::string Filename = SM.getFileEntryForID(ID)->getName().str();
  Injectors_.emplace(ID.getHashValue(),
                     std::make_unique<CodeInjector>(
                         Filename, generateOutputFilename(Filename)));
  Filenames_.emplace(ID.getHashValue(), std::move(Filename));
}

void InjectorASTWrapper::applySubstitutions() {
  for (auto& [Hash, Inj] : Injectors_) {
    Inj->applySubstitutions();
  }
}

void InjectorASTWrapper::substitute(Substitution&& Substr,
                                    const clang::ASTContext* Context) {
  const auto& SM = Context->getSourceManager();
  Injectors_[SM.getMainFileID().getHashValue()]->substitute(std::move(Substr));
}

void InjectorASTWrapper::substitute(const clang::SourceRange& Range,
                                    std::string NewString,
                                    const clang::ASTContext* Context) {
  SubstitutionASTWrapper(Context)
      .setLoc(Range.getBegin())
      .setFormats(getRangeAsString(Range, Context), NewString)
      .apply();
}

} // namespace ub_tester
