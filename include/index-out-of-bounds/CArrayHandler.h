#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

#include <string>
#include <utility>
#include <vector>

namespace ub_tester {

class CArrayHandler : public clang::RecursiveASTVisitor<CArrayHandler> {
public:
  explicit CArrayHandler(clang::ASTContext*);

  bool VisitFunctionDecl(clang::FunctionDecl*);

  bool VisitArrayType(clang::ArrayType*);

  bool VisitConstantArrayType(clang::ConstantArrayType*);

  bool VisitVariableArrayType(clang::VariableArrayType*);

  bool VisitIncompleteArrayType(clang::IncompleteArrayType*);

  bool VisitInitListExpr(clang::InitListExpr*);

  bool VisitStringLiteral(clang::StringLiteral*);

  bool TraverseVarDecl(clang::VarDecl*);

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr*);

private:
  void executeSubstitutionOfSubscript(clang::ArraySubscriptExpr*);
  void executeSubstitutionOfDecl(clang::VarDecl* ArrayDecl);

private:
  struct ArrayInfo_t {
    void reset();
    std::string Name_, Type_;
    std::vector<std::string> Sizes_;
    std::string InitList_;
    bool shouldVisitNodes_;
    bool isIncompleteType_, hasInitList_;
    bool isElementIsPointer_;
    std::string LowestLevelPointeeType_;
  };

private:
  ArrayInfo_t Array_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
