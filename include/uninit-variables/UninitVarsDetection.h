#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class FindFundTypeVarDeclVisitor : public clang::RecursiveASTVisitor<FindFundTypeVarDeclVisitor> {
public:
  explicit FindFundTypeVarDeclVisitor(clang::ASTContext* Context);
  bool VisitVarDecl(clang::VarDecl* VDecl);

private:
  clang::ASTContext* Context_;
};

class FindSafeTypeAccessesVisitor : public clang::RecursiveASTVisitor<FindSafeTypeAccessesVisitor> {
public:
  explicit FindSafeTypeAccessesVisitor(clang::ASTContext* Context);
  bool VisitDeclRefExpr(clang::DeclRefExpr* DRExpr);

private:
  clang::ASTContext* Context_;
};

class FindSafeTypeOperatorsVisitor : public clang::RecursiveASTVisitor<FindSafeTypeOperatorsVisitor> {
public:
  explicit FindSafeTypeOperatorsVisitor(clang::ASTContext* Context);
  bool VisitBinaryOperator(clang::BinaryOperator* Binop);
  bool VisitUnaryOperator(clang::UnaryOperator* Unop);

private:
  clang::ASTContext* Context_;
};

class FindUninitVarsConsumer : public clang::ASTConsumer {
public:
  explicit FindUninitVarsConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindFundTypeVarDeclVisitor FundamentalTypeVarDeclVisitor_;
  FindSafeTypeAccessesVisitor SafeTypeAccessesVisitor_;
  FindSafeTypeOperatorsVisitor SafeTypeOperatorsVisitor_;
};

} // namespace ub_tester