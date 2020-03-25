#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include "index-out-of-bounds/SafeSubstituterConsumer.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

class IndexOutOfBoundsAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new ub_tester::SubstituterConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(
      OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<IndexOutOfBoundsAction>().get());
}
