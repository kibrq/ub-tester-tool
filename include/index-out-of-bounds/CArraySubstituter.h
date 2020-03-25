#pragma once

#include "UBUtility.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class CArraySubstituter : public clang::RecursiveASTVisitor<CArraySubstituter> {
public:
  explicit CArraySubstituter(clang::ASTContext* Context);

  bool VisitFunctionDecl(clang::FunctionDecl* fd);

  bool VisitArrayType(clang::ArrayType* type);

  bool VisitConstantArrayType(clang::ConstantArrayType* type);

  bool VisitVariableArrayType(clang::VariableArrayType* type);

  bool VisitInitListExpr(clang::InitListExpr* initlist);

  bool VisitStringLiteral(clang::StringLiteral* sl);

  bool TraverseVarDecl(clang::VarDecl* vd);

  void HandleArrayDecl();

private:
  var_info_ array_;
  clang::ASTContext* Context;
};

} // namespace ub_tester
