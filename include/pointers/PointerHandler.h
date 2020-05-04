#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <string>
#include <utility>

namespace ub_tester {

class PointerHandler : public clang::RecursiveASTVisitor<PointerHandler> {
public:
  explicit PointerHandler(clang::ASTContext*);

  bool VisitPointerType(clang::PointerType*);

  bool VisitCXXNewExpr(clang::CXXNewExpr*);

  bool VisitCallExpr(clang::CallExpr*);

  bool TraverseDecl(clang::Decl*);

  bool VisitDeclRefExpr(clang::DeclRefExpr*);

  bool VisitBinaryOperator(clang::BinaryOperator*);

private:
  std::pair<std::string, std::string> getDeclFormats();
  std::pair<std::string, std::string> getSubscriptFormats();

private:
  void exeucteSubstituteOfPointerDecl();

private:
  struct PointerInfo_t {
    void reset();
    std::optional<std::string> Name_, Type_, Size_;
    size_t Dimension_;
    bool shouldVisitNodes_, shouldVisitDeclRefExpr_;
  };

private:
  clang::ASTContext* Context_;
  PointerInfo_t Pointer_;
};

} // namespace ub_tester
