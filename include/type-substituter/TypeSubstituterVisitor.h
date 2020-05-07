#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <sstream>

namespace ub_tester {
class TypeSubstituterVisitor
    : public clang::RecursiveASTVisitor<TypeSubstituterVisitor> {
public:
  explicit TypeSubstituterVisitor(clang::ASTContext*);

  bool TraverseVariableArrayType(clang::VariableArrayType*);
  bool TraverseDepedentSizedArrayType(clang::DependentSizedArrayType*);
  bool TraverseIncompleteArrayType(clang::IncompleteArrayType*);
  bool TraverseRValueReferenceType(clang::RValueReferenceType*);
  bool TraverseLValueReferenceType(clang::LValueReferenceType*);
  bool TraverseConstantArrayType(clang::ConstantArrayType*);
  bool TraversePointerType(clang::PointerType*);
  bool TraverseBuiltinType(clang::BuiltinType*);
  bool TraverseType(clang::QualType);

  bool TraverseDecl(clang::Decl*);

private:
  void substituteVarDeclType(clang::DeclaratorDecl*);
  void substituteReturnType(clang::FunctionDecl*);
  bool TraverseArrayType(clang::ArrayType*);

private:
  struct TypeInfo_t {
    void reset();
    std::stringstream& getName();
    const std::stringstream& getName() const;
    bool isFirstInit() const;
    bool isInited() const;
    void shouldVisitTypes(bool flag);
    bool shouldVisitTypes();

    void addConst();

  private:
    void init();

  private:
    std::optional<std::stringstream> Name_{std::nullopt};
    bool shouldVisitTypes_{false};
    bool isQualPrev{false};
    int FirstInit_{0};
  };

private:
  TypeInfo_t Type_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
