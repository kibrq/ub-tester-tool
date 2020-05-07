#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ub_tester {

class CArrayHandler : public clang::RecursiveASTVisitor<CArrayHandler> {
public:
  explicit CArrayHandler(clang::ASTContext*);

  bool shouldVisitImplicitCode();

  bool VisitConstantArrayType(clang::ConstantArrayType*);

  bool VisitVariableArrayType(clang::VariableArrayType*);

  bool VisitIncompleteArrayType(clang::IncompleteArrayType*);

  bool VisitInitListExpr(clang::InitListExpr*);

  bool VisitStringLiteral(clang::StringLiteral*);

  bool TraverseVarDecl(clang::VarDecl*);

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr*);

private:
  std::pair<std::string, std::string> getCtorFormats();
  std::pair<std::string, std::string> getSubscriptFormats();

private:
  void executeSubstitutionOfSubscript(clang::ArraySubscriptExpr*);
  void executeSubstitutionOfCtor(clang::VarDecl*);

private:
  struct ArrayInfo_t {
    void reset();
    std::optional<std::string> Init_;
    std::vector<std::string> Sizes_;
    size_t Dimension_;
    bool shouldVisitNodes_, isIncompleteType_, shouldVisitImplicitCode_;
  };

private:
  ArrayInfo_t Array_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
