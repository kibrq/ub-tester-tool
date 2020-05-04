#include "utility/GetFuncCodeAvailVisitor.h"

#include "clang/AST/ASTContext.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

#include "UBUtility.h"
#include <iostream>

namespace ub_tester {

GetFuncCodeAvailVisitor::GetFuncCodeAvailVisitor(clang::ASTContext* Context) : Context(Context) {}

bool GetFuncCodeAvailVisitor::VisitFunctionDecl(FunctionDecl* FuncDecl) {
  if (FuncDecl->doesThisDeclarationHaveABody())
    func_code_avail::setHasFuncAvailCode(FuncDecl);
  return true;
}

} // namespace ub_tester