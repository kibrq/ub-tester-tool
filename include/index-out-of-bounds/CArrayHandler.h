#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ub_tester {

class CArrayVisitor : public clang::RecursiveASTVisitor<CArrayVisitor> {
public:
  explicit CArrayVisitor(clang::ASTContext*);

  bool shouldVisitImplicitCode();

  bool VisitConstantArrayType(clang::ConstantArrayType*);
  bool VisitVariableArrayType(clang::VariableArrayType*);
  bool VisitIncompleteArrayType(clang::IncompleteArrayType*);
  bool VisitInitListExpr(clang::InitListExpr*);
  bool VisitStringLiteral(clang::StringLiteral*);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr*);

  bool TraverseVarDecl(clang::VarDecl*);

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
    bool ShouldVisitNodes_, IsIncompleteType_, ShouldVisitImplicitCode_;
  };

private:
  ArrayInfo_t Array_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
