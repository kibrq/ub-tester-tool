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
#include "code-injector/ASTFrontendInjector.h"
#include "index-out-of-bounds/IOBConsumer.h"
#include "type-substituter/TypeSubstituterConsumer.h"
#include "uninit-variables/UninitVarsDetection.h"
#include "utility/UtilityConsumer.h"

#include <experimental/filesystem>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

namespace ub_tester {
class UBTesterAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {

    std::unique_ptr<ASTConsumer> OutOfBoundsConsumer = std::make_unique<IOBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> UninitVarsConsumer = std::make_unique<AssertUninitVarsConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> ArithmeticUBConsumer = std::make_unique<FindArithmeticUBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> TypeSubstituter = std::make_unique<TypeSubstituterConsumer>(&Compiler.getASTContext());

    std::vector<std::unique_ptr<ASTConsumer>> consumers;
    // consumers.emplace_back(std::move(OutOfBoundsConsumer));
    consumers.emplace_back(std::move(UninitVarsConsumer));
    consumers.emplace_back(std::move(ArithmeticUBConsumer));
    consumers.emplace_back(std::move(TypeSubstituter));

    return std::make_unique<MultiplexConsumer>(std::move(consumers));
  }
};

class UBTesterUtilityAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::make_unique<UtilityConsumer>(&Compiler.getASTContext());
  }
};

} // namespace ub_tester

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

  ub_tester::ASTFrontendInjector::initialize(OptionsParser.getSourcePathList());
  ub_tester::ASTFrontendInjector::getInstance().substituteIncludePaths();

  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  int UtilityReturnCode = Tool.run(newFrontendActionFactory<ub_tester::UBTesterUtilityAction>().get());
  if (UtilityReturnCode) {
    std::cerr << "File(s) preprocessing failed\n";
    exit(1);
  }

  int ReturnCode = Tool.run(newFrontendActionFactory<ub_tester::UBTesterAction>().get());
  if (!ReturnCode) {
    ub_tester::ASTFrontendInjector::getInstance().applySubstitutions();
  }
}
