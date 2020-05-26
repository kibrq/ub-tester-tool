#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include "arithmetic-overflow/ArithmeticUBAsserts.h"
#include "arithmetic-overflow/FindArithmeticUBConsumer.h"
#include "cli/CLIOptions.h"
#include "code-injector/ASTFrontendInjector.h"
#include "index-out-of-bounds/IOBConsumer.h"
#include "type-substituter/TypeSubstituterConsumer.h"
#include "uninit-variables/UninitVarsDetection.h"

#include <experimental/filesystem>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory UBTesterOptionsCategory("ub-tester options");
// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
// static cl::extrahelp MoreHelp("\nMore help text...\n");

namespace ub_tester {

namespace clio {

bool runOOB;
bool runArithm;
bool runUninit;
bool SuppressWarnings;

namespace internal {
ApplyOnly AO;

static cl::opt<bool, true> SuppressWarningsFlag("no-warn", cl::desc("Disable warnings output"),
                                                cl::location(ub_tester::clio::SuppressWarnings), cl::init(false),
                                                cl::cat(UBTesterOptionsCategory));
static cl::opt<ApplyOnly, true>
    ApplyOnlyOption("apply-only", cl::desc("Only apply specified checks"),
                    cl::values(clEnumValN(ApplyOnly::OOB, "oob", "Arrays and pointers out of bounds checks"),
                               clEnumValN(ApplyOnly::Arithm, "arithm", "Arithmetic operations checks"),
                               clEnumValN(ApplyOnly::Uninit, "uninit", "Uninitialized variables checks")),
                    cl::location(AO), cl::init(ApplyOnly::All), cl::cat(UBTesterOptionsCategory));
} // namespace internal
} // namespace clio

class UBTesterAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {

    std::unique_ptr<ASTConsumer> OutOfBoundsConsumer = std::make_unique<IOBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> UninitVarsConsumer = std::make_unique<AssertUninitVarsConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> ArithmeticUBConsumer = std::make_unique<FindArithmeticUBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> TypeSubstituter = std::make_unique<TypeSubstituterConsumer>(&Compiler.getASTContext());

    std::vector<std::unique_ptr<ASTConsumer>> consumers;
    if (clio::runOOB)
      consumers.emplace_back(std::move(OutOfBoundsConsumer));
    if (clio::runUninit)
      consumers.emplace_back(std::move(UninitVarsConsumer));
    if (clio::runArithm)
      consumers.emplace_back(std::move(ArithmeticUBConsumer));
    consumers.emplace_back(std::move(TypeSubstituter));

    return std::make_unique<MultiplexConsumer>(std::move(consumers));
  }
};
} // namespace ub_tester

void UBTesterVersionPrinter(raw_ostream& ostr) {
  ostr << "ub-tester tool\n\nVersion: b0.9\n\nAuthors: https://github.com/KirillBrilliantov, https://github.com/GlebSolovev, "
          "https://github.com/DLochmelis33\n";
}

int main(int argc, const char** argv) {
  cl::SetVersionPrinter(UBTesterVersionPrinter);
  CommonOptionsParser OptionsParser(argc, argv, UBTesterOptionsCategory, cl::Optional);
  ub_tester::clio::processFlags();

  ub_tester::ASTFrontendInjector::initialize(OptionsParser.getSourcePathList());
  ub_tester::ASTFrontendInjector::getInstance().substituteIncludePaths();

  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  int ReturnCode = Tool.run(newFrontendActionFactory<ub_tester::UBTesterAction>().get());
  if (!ReturnCode) {
    ub_tester::ASTFrontendInjector::getInstance().applySubstitutions();
  }
}
