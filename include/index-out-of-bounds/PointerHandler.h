#pragma once

#include "UBUtility.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class PointerHandler : public clang::RecursiveASTVisitor<PointerHandler> {
public:
  explicit PointerHandler(clang::ASTContext* Context);

  bool VisitFunctionDecl(clang::FunctionDecl* fd);

  bool VisitPointerType(clang::PointerType* pt);

  bool VisitCXXNewExpr(clang::CXXNewExpr* cne);

  bool VisitCallExpr(clang::CallExpr* ce);

  bool TraverseVarDecl(clang::VarDecl* vd);

  bool VisitDeclRefExpr(clang::DeclRefExpr* ref_expr);

  bool VisitBinaryOperator(clang::BinaryOperator* bo);

private:
  struct PointerInfo_t {
    void reset();
    std::string Name_, Type_, Size_;
    bool shouldVisitNodes_, shouldVisitDeclRefExpr_;
  };

private:
  clang::ASTContext* Context;
  PointerInfo_t Pointer_;
};

} // namespace ub_tester
