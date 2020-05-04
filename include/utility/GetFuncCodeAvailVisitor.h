#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

namespace ub_tester {

class GetFuncCodeAvailVisitor : public clang::RecursiveASTVisitor<GetFuncCodeAvailVisitor> {
public:
  explicit GetFuncCodeAvailVisitor(clang::ASTContext* Context);

  // detect variable usage
  bool VisitFunctionDecl(clang::FunctionDecl* FuncDecl);

private:
  clang::ASTContext* Context;
};

}