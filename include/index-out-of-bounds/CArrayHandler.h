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

  bool VisitArrayType(clang::ArrayType*);

  bool VisitConstantArrayType(clang::ConstantArrayType*);

  bool VisitVariableArrayType(clang::VariableArrayType*);

  bool VisitIncompleteArrayType(clang::IncompleteArrayType*);

  bool VisitInitListExpr(clang::InitListExpr*);

  bool VisitStringLiteral(clang::StringLiteral*);

  bool TraverseParmVarDecl(clang::ParmVarDecl*);

  bool TraverseVarDecl(clang::VarDecl*);

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr*);

private:
  std::pair<std::string, std::string> getDeclFormats(bool isStatic, bool needCtor, char EndSymb);
  std::pair<std::string, std::string> getSubscriptFormats();

private:
  void executeSubstitutionOfSubscript(clang::ArraySubscriptExpr*);
  void executeSubstitutionOfArrayDecl(clang::SourceLocation BeginLoc, bool isStatic, bool needCtor,
                                      char EndSymb);
  void executeSubstitutionOfArrayDecl(clang::VarDecl* ArrayDecl);
  void executeSubstitutionOfArrayDecl(clang::ParmVarDecl* ArrayDecl);

private:
  struct ArrayInfo_t {
    void reset();
    std::optional<std::string> Name_;
    std::optional<std::string> Type_, LowestLevelPointeeType_;
    std::optional<std::string> InitList_;
    std::vector<std::string> Sizes_;
    size_t Dimension_;
    bool shouldVisitNodes_, isIncompleteType_;
  };

private:
  ArrayInfo_t Array_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
