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
#include "uninit-variables/UninitVarsDetection.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

namespace ub_tester {
class UBTesterAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler,
                                                                llvm::StringRef InFile) {
    ASTFrontendInjector::getInstance().addFile(&Compiler.getASTContext());

    std::unique_ptr<ASTConsumer> OutOfBoundsConsumer =
        std::make_unique<IOBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> UninitVarsConsumer =
        std::make_unique<AssertUninitVarsConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> ArithmeticUBConsumer =
        std::make_unique<FindArithmeticUBConsumer>(&Compiler.getASTContext());

    std::vector<std::unique_ptr<ASTConsumer>> consumers;
    consumers.emplace_back(std::move(OutOfBoundsConsumer));
    consumers.emplace_back(std::move(UninitVarsConsumer));
    consumers.emplace_back(std::move(ArithmeticUBConsumer));

    return std::make_unique<MultiplexConsumer>(std::move(consumers));
  }
};
} // namespace ub_tester

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<ub_tester::UBTesterAction>().get());
}
