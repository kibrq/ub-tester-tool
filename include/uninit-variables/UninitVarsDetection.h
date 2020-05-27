#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

class FindFundTypeVarDeclVisitor : public clang::RecursiveASTVisitor<FindFundTypeVarDeclVisitor> {
public:
  explicit FindFundTypeVarDeclVisitor(clang::ASTContext* Context);

  // substitute types to safe_type template
  bool VisitVarDecl(clang::VarDecl* varDecl);

private:
  clang::ASTContext* Context;
};

class FindSafeTypeAccessesVisitor : public clang::RecursiveASTVisitor<FindSafeTypeAccessesVisitor> {
public:
  explicit FindSafeTypeAccessesVisitor(clang::ASTContext* Context);

  // detect variable usage
  bool VisitDeclRefExpr(clang::DeclRefExpr* DRE);

private:
  clang::ASTContext* Context;
};

class FindSafeTypeOperatorsVisitor : public clang::RecursiveASTVisitor<FindSafeTypeOperatorsVisitor> {
public:
  explicit FindSafeTypeOperatorsVisitor(clang::ASTContext* Context);

  // detect variable assignment and more
  bool VisitBinaryOperator(clang::BinaryOperator* BinOp);
  bool VisitUnaryOperator(clang::UnaryOperator* UnOp);

private:
  clang::ASTContext* Context;
};

class AssertUninitVarsConsumer : public clang::ASTConsumer {
public:
  explicit AssertUninitVarsConsumer(clang::ASTContext* Context);

  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindFundTypeVarDeclVisitor FundamentalTypeVarDeclVisitor;
  FindSafeTypeAccessesVisitor SafeTypeAccessesVisitor;
  FindSafeTypeOperatorsVisitor SafeTypeOperatorsVisitor;
};

} // namespace ub_tester