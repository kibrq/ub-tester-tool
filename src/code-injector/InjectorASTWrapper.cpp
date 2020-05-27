#include "code-injector/InjectorASTWrapper.h"
#include "UBUtility.h"
#include "clang/Basic/SourceManager.h"
#include <experimental/filesystem>
#include <fstream>
#include <string_view>
#include <unordered_set>

using namespace clang;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;
namespace fs = std::experimental::filesystem;

namespace ub_tester::code_injector::wrapper {

InjectorASTWrapper& InjectorASTWrapper::getInstance() {
  static InjectorASTWrapper Injector;
  return Injector;
}

namespace {

constexpr char UBTesterPrefix[] = "IMPROVED_";
constexpr char IncludeKeyword[] = "#include";

std::string generateOutputFilename(std::string Filename) {
  fs::path Path{std::move(Filename)};
  Path.replace_filename(UBTesterPrefix +
                        static_cast<std::string>(Path.filename()));
  return static_cast<std::string>(Path);
}

} // namespace

void InjectorASTWrapper::substituteIncludePaths(
    const std::vector<std::string>& Files) {
  std::unordered_set<std::string> AvailFilenames;
  auto& InternalInjectors = getInstance().InternalInjectors_;
  for (const auto& Filename : Files) {
    AvailFilenames.insert(
        static_cast<std::string>(fs::path{Filename}.filename()));
  }
  for (const auto& Inj : InternalInjectors) {
    std::ifstream IStream(Inj->getInputFilename(), std::ios::in);
    std::string Word;
    bool IsIncludePrev = false;
    while (IStream >> Word) {
      if (Word.compare(IncludeKeyword) == 0) {
        IsIncludePrev = true;
        continue;
      }
      if (!IsIncludePrev)
        continue;
      // Don't really know why Word.length() - 2...
      if (std::string IncludeFilename = static_cast<std::string>(
              fs::path(Word.substr(1, Word.length() - 2)).filename());
          AvailFilenames.find(IncludeFilename) != AvailFilenames.end()) {
        size_t Pos = IStream.tellg();
        Inj->substitute(Pos - Word.length(), SubstPriorityKind::Medium, "$@",
                        UBTesterPrefix + std::string{"@"}, {IncludeFilename});
      }
      IsIncludePrev = false;
    }
  }
}

void InjectorASTWrapper::addFile(const ASTContext* Context) {
  const auto& SrcManager = Context->getSourceManager();
  auto Id = SrcManager.getMainFileID();
  std::string Filename = SrcManager.getFileEntryForID(Id)->getName().str();
  InternalInjectors_.emplace_back(std::make_unique<CodeInjector>(
      Filename, generateOutputFilename(Filename)));
}

void InjectorASTWrapper::applySubstitutions() {
  for (auto& Inj : InternalInjectors_)
    Inj->applySubstitutions();
}

void InjectorASTWrapper::substitute(Substitution Substr,
                                    const clang::ASTContext* Context) {
  InternalInjectors_.back()->substitute(std::move(Substr));
}

void InjectorASTWrapper::substitute(const clang::SourceRange& Range,
                                    std::string NewString,
                                    const clang::ASTContext* Context) {
  SubstitutionASTWrapper(Context)
      .setLoc(Range.getBegin())
      .setFormats(getRangeAsString(Range, Context), std::move(NewString))
      .apply();
}

} // namespace ub_tester::code_injector::wrapper
