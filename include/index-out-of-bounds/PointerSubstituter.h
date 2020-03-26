#pragma once

#include "UBUtility.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class PointerSubstituter
    : public clang::RecursiveASTVisitor<PointerSubstituter> {
public:
  explicit PointerSubstituter(clang::ASTContext* Context);

  bool VisitFunctionDecl(clang::FunctionDecl* fd);

  bool VisitPointerType(clang::PointerType* pt);

  bool VisitCXXNewExpr(clang::CXXNewExpr* cne);

  bool VisitCallExpr(clang::CallExpr* ce);

  bool TraverseVarDecl(clang::VarDecl* vd);

  bool VisitDeclRefExpr(clang::DeclRefExpr* ref_expr);

  bool VisitBinaryOperator(clang::BinaryOperator* bo);

private:
  clang::ASTContext* Context;
  var_info_ pointer_;
  bool should_visit_decl_ref_expr_;
};

} // namespace ub_tester
