#pragma once
#include "clang/AST/RecursiveASTVisitor.h"

#include <string>
#include <vector>

#include "UBUtility.h"

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

private:
  std::string generateSafeType(clang::VarDecl* ArrayVarDecl);

private:
  struct ArrayInfo_t {
    void reset();
    std::string Name_, Type_;
    std::vector<std::string> Sizes_;
    std::string InitList_;
    bool isIncompleteType_, shouldVisitNodes_, hasInitList_;
  };

private:
  ArrayInfo_t Array_;
  clang::ASTContext* Context;
};

} // namespace ub_tester
