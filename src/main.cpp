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
#include "code-injector/InjectorASTWrapper.h"
#include "index-out-of-bounds/IOBConsumer.h"
#include "pointers/PointersConsumer.h"
#include "type-substituter/TypeSubstituterConsumer.h"
#include "uninit-variables/UninitVarsDetection.h"
#include "utility/UtilityConsumer.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

namespace ub_tester {
class UBTesterAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {

    InjectorASTWrapper::getInstance().addFile(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> OutOfBoundsConsumer =
        std::make_unique<IOBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> UninitVarsConsumer =
        std::make_unique<AssertUninitVarsConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> ArithmeticUBConsumer =
        std::make_unique<FindArithmeticUBConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> TypeSubstituter =
        std::make_unique<TypeSubstituterConsumer>(&Compiler.getASTContext());
    std::unique_ptr<ASTConsumer> PointerConsumer =
        std::make_unique<PointersConsumer>(&Compiler.getASTContext());

    std::vector<std::unique_ptr<ASTConsumer>> consumers;
    consumers.emplace_back(std::move(OutOfBoundsConsumer));
    consumers.emplace_back(std::move(UninitVarsConsumer));
    consumers.emplace_back(std::move(ArithmeticUBConsumer));
    consumers.emplace_back(std::move(TypeSubstituter));
    consumers.emplace_back(std::move(PointerConsumer));

    return std::make_unique<MultiplexConsumer>(std::move(consumers));
  }
};

class UBTesterUtilityAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::make_unique<UtilityConsumer>(&Compiler.getASTContext());
  }
};

} // namespace ub_tester

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

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
