#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <sstream>

namespace ub_tester {
class TypeSubstituterVisitor
    : public clang::RecursiveASTVisitor<TypeSubstituterVisitor> {
public:
  explicit TypeSubstituterVisitor(clang::ASTContext*);

  bool TraverseBuiltinTypeLoc(clang::BuiltinTypeLoc);
  bool TraverseConstantArrayTypeLoc(clang::ConstantArrayTypeLoc);
  bool TraverseVariableArrayTypeLoc(clang::VariableArrayTypeLoc);
  bool TraverseDepedentSizedArrayTypeLoc(clang::DependentSizedArrayTypeLoc);
  bool TraverseIncompleteArrayTypeLoc(clang::IncompleteArrayTypeLoc);
  bool TraversePointerTypeLoc(clang::PointerTypeLoc);

  bool TraverseDecl(clang::Decl*);

private:
  struct TypeInfo_t {
    void init();
    void reset();
    std::stringstream& getName();
    const std::stringstream& getName() const;
    bool isFirstInit() const;
    bool isInited() const;
    void shouldVisitTypes(bool flag);
    bool shouldVisitTypes();

  private:
    std::optional<std::stringstream> Name_{std::nullopt};
    bool shouldVisitTypes_{false};
    int FirstInit_{0};
  };

private:
  TypeInfo_t Type_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
