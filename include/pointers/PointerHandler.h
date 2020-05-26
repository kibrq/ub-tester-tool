#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ub_tester {

class PointerHandler : public clang::RecursiveASTVisitor<PointerHandler> {
public:
  explicit PointerHandler(clang::ASTContext*);

  bool VisitCallExpr(clang::CallExpr*);
  bool VisitCXXNewExpr(clang::CXXNewExpr*);
  bool TraverseVarDecl(clang::VarDecl*);
  bool TraverseBinAssign(clang::BinaryOperator*, DataRecursionQueue* = nullptr);

  bool VisitUnaryOperator(clang::UnaryOperator*);
  bool VisitMemberExpr(clang::MemberExpr*);

private:
  std::pair<std::string, std::string> getCtorFormats();
  std::pair<std::string, std::string> getAssignFormats();

private:
  void executeSubstitutionOfPointerCtor(clang::VarDecl*);
  void executeSubstitutionOfPointerAssignment(clang::BinaryOperator*);
  void executeSubstitutionOfStarOperator(clang::UnaryOperator*);
  void executeSubstitutionOfMemberExpr(clang::MemberExpr*);

private:
  struct PointerInfo_t {
    PointerInfo_t(bool);
    std::optional<std::string> Init_{std::nullopt};
    std::string PointeeType_;
    std::stringstream Size_;
    bool shouldVisitNodes_;
    bool hasSize_{false};
  };

private:
  bool shouldVisitNodes();
  void reset();
  PointerInfo_t& backPointer();

private:
  clang::ASTContext* Context_;
  std::vector<PointerInfo_t> Pointers_;
};

} // namespace ub_tester
