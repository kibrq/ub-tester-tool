#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>

#include "arithmetic-ub/FindArithmeticUBConsumer.h"
#include "cli/CLI.h"
#include "code-injector/InjectorASTWrapper.h"
#include "index-out-of-bounds/FindIOBConsumer.h"
#include "pointer-ub/FindPointerUBConsumer.h"
#include "type-substituter/TypeSubstituterConsumer.h"
#include "uninit-variables/UninitVarsDetection.h"

#include <iostream>
#include <vector>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

static llvm::cl::OptionCategory UBTesterOptionsCategory("ub-tester options");
// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
// static cl::extrahelp MoreHelp("\nMore help text...\n");

namespace ub_tester {

namespace cli {

bool RunIOB;
bool RunArithm;
bool RunUninit;
bool SuppressWarnings;
bool SuppressAllOutput;

namespace internal {

ApplyOnly CheckToApply;

static cl::opt<bool, true>
    SuppressWarningsFlag("no-warn", cl::desc("Disable warnings output"),
                         cl::location(SuppressWarnings), cl::init(false),
                         cl::cat(UBTesterOptionsCategory));
static cl::alias SuppressWarningsFlagAlias("w", cl::desc("Alias for -no-warn"),
                                           cl::aliasopt(SuppressWarningsFlag),
                                           cl::cat(UBTesterOptionsCategory));
static cl::opt<ApplyOnly, true> ApplyOnlyOption(
    "apply-only", cl::desc("Only apply specified checks"),
    cl::values(clEnumValN(ApplyOnly::IOB, "iob", "Index out of bounds checks"),
               clEnumValN(ApplyOnly::Arithm, "arithm",
                          "Arithmetic operations checks"),
               clEnumValN(ApplyOnly::Uninit, "uninit",
                          "Uninitialized variables checks")),
    cl::location(CheckToApply), cl::init(ApplyOnly::All),
    cl::cat(UBTesterOptionsCategory));

static cl::opt<bool, true>
    SuppressAllOutputFlag("quiet",
                          cl::desc("Disable all output, exit program on error"),
                          cl::location(SuppressAllOutput), cl::init(false),
                          cl::cat(UBTesterOptionsCategory));
static cl::alias SuppressAllOutputFlagAlias("q", cl::desc("Alias for -quiet"),
                                            cl::aliasopt(SuppressAllOutputFlag),
                                            cl::cat(UBTesterOptionsCategory));
} // namespace internal
} // namespace cli

class UBTesterAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {

    InjectorASTWrapper::getInstance().addFile(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> IOBConsumer =
        std::make_unique<FindIOBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> UninitVarsConsumer =
        std::make_unique<FindUninitVarsConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> ArithmeticUBConsumer =
        std::make_unique<FindArithmeticUBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> TypeSubstituter =
        std::make_unique<TypeSubstituterConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> PointerUBConsumer =
        std::make_unique<FindPointerUBConsumer>(&Compiler.getASTContext());

    std::vector<std::unique_ptr<ASTConsumer>> consumers;
    if (cli::RunIOB) {
      consumers.emplace_back(std::move(IOBConsumer));
      consumers.emplace_back(std::move(PointerUBConsumer));
    }
    if (cli::RunUninit)
      consumers.emplace_back(std::move(UninitVarsConsumer));
    if (cli::RunArithm)
      consumers.emplace_back(std::move(ArithmeticUBConsumer));
    consumers.emplace_back(std::move(TypeSubstituter));

    return std::make_unique<MultiplexConsumer>(std::move(consumers));
  }
};

class UBTesterUtilityAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::make_unique<util::func_code_avail::UtilityConsumer>(
        &Compiler.getASTContext());
  }
};

} // namespace ub_tester

void UBTesterVersionPrinter(raw_ostream& OStream) {
  OStream << "ub-tester tool\n"
             "Change input programs so that they exit before some cases of UB "
             "to prevent it\n"
             "\n"
             "Version: b1.0\n"
             "\n"
             "Authors: https://github.com/KirillBrilliantov, "
             "https://github.com/GlebSolovev, "
             "https://github.com/DLochmelis33\n";
}

int main(int argc, const char** argv) {
  cl::SetVersionPrinter(UBTesterVersionPrinter);
  CommonOptionsParser OptionsParser(argc, argv, UBTesterOptionsCategory,
                                    cl::ZeroOrMore);
  ub_tester::cli::processFlags();

  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  int UtilityReturnCode = Tool.run(
      newFrontendActionFactory<ub_tester::UBTesterUtilityAction>().get());
  if (UtilityReturnCode) {
    std::cerr << "File(s) preprocessing failed\n";
    exit(1);
  }

  int ReturnCode =
      Tool.run(newFrontendActionFactory<ub_tester::UBTesterAction>().get());

  if (!ReturnCode) {
    InjectorASTWrapper::getInstance().substituteIncludePaths(
        OptionsParser.getSourcePathList());
    InjectorASTWrapper::getInstance().applySubstitutions();
  }
}
