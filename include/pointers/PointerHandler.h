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
  bool TraverseVarDecl(clang::VarDecl*);
  bool TraverseBinAssign(clang::BinaryOperator*, DataRecursionQueue* = nullptr);

private:
  std::pair<std::string, std::string> getCtorFormats();
  std::pair<std::string, std::string> getAssignFormats();
  std::pair<std::string, std::string> getSubscriptFormats();

private:
  void executeSubstitutionOfPointerCtor(clang::VarDecl*);
  void executeSubstitutionOfPointerAssignment(clang::BinaryOperator*);

private:
  struct PointerInfo_t {
    PointerInfo_t(bool);
    std::optional<std::string> Init_;
    std::string PointeeType_;
    std::stringstream Size_;
    bool shouldVisitNodes_;
  };

private:
  bool shouldVisitNodes();
  void reset();

private:
  clang::ASTContext* Context_;
  std::vector<PointerInfo_t> Pointers_;
};

} // namespace ub_tester
