#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <stdio.h>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

class PointerAndArrSubstituter
    : public RecursiveASTVisitor<PointerAndArrSubstituter> {
public:
  explicit PointerAndArrSubstituter(ASTContext* Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl* fd) {
    if (Context->getSourceManager().isInMainFile(fd->getBeginLoc())) {
      for (const auto& param : fd->parameters()) {
        auto type = param->getOriginalType().getTypePtrOrNull();
        if (type && type->isPointerType() && !type->isFunctionPointerType()) {
          printf("Pointer in fd\n");
        }
      }
    }
    return true;
  }

  bool VisitVarDecl(VarDecl* vd) {
    if (Context->getSourceManager().isInMainFile(vd->getBeginLoc())) {
      auto type = vd->getType().getTypePtrOrNull();
      if (type && ((type->isPointerType() && !type->isFunctionPointerType()) ||
                   type->isArrayType())) {
        printf("Pointer or array in ds\n");
      }
    }
    return true;
  }

private:
  ASTContext* Context;
};

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext* Context) : Context(Context) {}

  bool TraverseDeclStmt(DeclStmt* ds) {
    printf("Hello\n");
    RecursiveASTVisitor<FindNamedClassVisitor>::TraverseDeclStmt(ds);
    if (malloced) {
      printf("Malloced\n");
    }
    malloced = false;
    return true;
  }

  bool VisitCallExpr(CallExpr* ce) {
    if (Context->getSourceManager().isInMainFile(ce->getBeginLoc())) {
      auto func_name = ce->getDirectCallee()->getNameInfo().getAsString();
      malloced = true;
    }
    return true;
  }

  bool VisitArraySubscriptExpr(ArraySubscriptExpr* ase) {
    if (Context->getSourceManager().isInMainFile(ase->getBeginLoc())) {
      auto arr_name = ase->getBase();
      StringRef sf_arr_name = Lexer::getSourceText(
          CharSourceRange::getTokenRange(arr_name->getSourceRange()),
          Context->getSourceManager(), Context->getLangOpts());
      auto index_name = ase->getIdx();
      StringRef sf_index_name = Lexer::getSourceText(
          CharSourceRange::getTokenRange(index_name->getSourceRange()),
          Context->getSourceManager(), Context->getLangOpts());
      printf(
          "ASSERT(%s, sizeof(%s)/sizeof(%s[0]))\n", sf_index_name.str().c_str(),
          sf_arr_name.str().c_str(), sf_arr_name.str().c_str());
      printf(
          "Begins at %u\n", Context->getSourceManager().getSpellingColumnNumber(
                                index_name->getBeginLoc()));
    }

    return true;
  }

private:
  bool malloced = false;
  ASTContext* Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext* Context)
      : Substituter(Context), Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext& Context) {
    Substituter.TraverseDecl(Context.getTranslationUnitDecl());
    // Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  PointerAndArrSubstituter Substituter;
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new FindNamedClassConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(
      OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<FindNamedClassAction>().get());
}
