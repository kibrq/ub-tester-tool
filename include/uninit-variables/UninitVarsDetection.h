#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <sstream>
#include <stdexcept>
#include <string>
// this only needs to be included in target file; no other use
// TODO: require <string> or change to c-like string
template <typename T> class UB_UninitSafeType {
public:
  UB_UninitSafeType() : value{}, isInit{false}, isIgnored{false} {}
  UB_UninitSafeType(T t) : value{t}, isInit{true}, isIgnored{false} {}

  T& getValue(std::string varName = std::string(), size_t line = 0) {
    if (!isIgnored && !isInit) {
      std::stringstream errorMessage{"access to value of uninitialized variable"};
      if (varName != "")
        errorMessage << ' ' << varName;
      if (line != 0)
        errorMessage << " at " << line;
      throw std::logic_error(errorMessage.str());
    }
    return value;
  }
  T& getIgnore() {
    isIgnored = true;
    return value;
  }
  T& tryInitValue(T t) {
    value = t;
    isInit = true;
    return value;
  }

private:
  T value;
  bool isInit;
  bool isIgnored;
};

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
  bool VisitImplicitCastExpr(clang::ImplicitCastExpr* ice);

private:
  clang::ASTContext* Context;
};

class FindSafeTypeDefinitionsVisitor : public clang::RecursiveASTVisitor<FindSafeTypeDefinitionsVisitor> {
public:
  explicit FindSafeTypeDefinitionsVisitor(clang::ASTContext* Context);

  // detect variable usage
  bool VisitBinaryOperator(clang::BinaryOperator* BinOp);

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