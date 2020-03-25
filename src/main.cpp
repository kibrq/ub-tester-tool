#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <cassert>
#include <sstream>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

std::string getExprName(Expr* Expression, ASTContext* Context) {
  StringRef Name = Lexer::getSourceText(
      CharSourceRange::getTokenRange(Expression->getSourceRange()),
      Context->getSourceManager(), Context->getLangOpts());
  return Name.str();
}

std::string getExprLineNCol(Expr* Expression, ASTContext* Context) {
  FullSourceLoc FullLocation = Context->getFullLoc(Expression->getBeginLoc());
  std::stringstream res;
  res << FullLocation.getSpellingLineNumber() << ":"
      << FullLocation.getSpellingColumnNumber();
  if (FullLocation.isValid())
    return res.str();
  return "invalid location";
}

class FindArithmeticOverflowVisitor
    : public RecursiveASTVisitor<FindArithmeticOverflowVisitor> {
public:
  explicit FindArithmeticOverflowVisitor(ASTContext* Context)
      : Context(Context) {}

  bool VisitBinaryOperator(BinaryOperator* Binop) {
    if (!(Binop->isAdditiveOp() || Binop->isMultiplicativeOp()))
      return true;
    Expr* Lhs = Binop->getLHS();
    Expr* Rhs = Binop->getRHS();
    QualType LhsType = Lhs->getType().getUnqualifiedType();
    QualType RhsType = Rhs->getType().getUnqualifiedType();

    if (LhsType.isTrivialType(*Context)) {
      assert(RhsType.isTrivialType(*Context));
      assert(LhsType.getAsString() == RhsType.getAsString());
      //llvm::outs() << LhsType.getAsString() << " is trivial\n";

      std::string AssertName = "undefined";
      if (Binop->isAdditiveOp())
        AssertName = "ASSERT_SUM";
      else if (Binop->isMultiplicativeOp())
        AssertName = "ASSERT_MUL";

      llvm::outs() << getExprLineNCol(Binop, Context) << " " << AssertName
                   << "<" << LhsType.getAsString() << ">("
                   << getExprName(Lhs, Context) << ", "
                   << getExprName(Rhs, Context) << ");\n";
    }

    return true;
  }

private:
  ASTContext* Context;
};

class FindArithmeticOverflowConsumer : public clang::ASTConsumer {
public:
  explicit FindArithmeticOverflowConsumer(ASTContext* Context)
      : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext& Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindArithmeticOverflowVisitor Visitor;
};

class FindArithmeticOverflowAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new FindArithmeticOverflowConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(
      newFrontendActionFactory<FindArithmeticOverflowAction>().get());
}
