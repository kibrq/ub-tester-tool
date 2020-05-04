#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace ub_tester {

namespace UB_UninitSafeTypeConsts {
const std::string TEMPLATE_NAME = "UB_UninitSafeType";
const std::string GETMETHOD_NAME = "getValue";
const std::string INITMETHOD_NAME = "tryInitValue";
} // namespace UB_UninitSafeTypeConsts

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
  bool VisitImplicitCastExpr(clang::ImplicitCastExpr* ice);

private:
  clang::ASTContext* Context;
};

class FindSafeTypeDefinitionsVisitor : public clang::RecursiveASTVisitor<FindSafeTypeDefinitionsVisitor> {
public:
  explicit FindSafeTypeDefinitionsVisitor(clang::ASTContext* Context);

  // detect variable usage
  bool VisitBinaryOperator(clang::BinaryOperator* BinOp);
  // dummy
  bool VisitCallExpr(clang::CallExpr* ce);

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
  FindSafeTypeDefinitionsVisitor SafeTypeDefinitionsVisitor;
};

} // namespace ub_tester