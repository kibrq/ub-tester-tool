#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

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
  bool TraverseRecordType(clang::RecordType*);
  bool TraverseEnumType(clang::EnumType*);
  bool TraverseTemplateSpecializationType(clang::TemplateSpecializationType*);
  bool TraverseType(clang::QualType);

  bool VisitDeclStmt(clang::DeclStmt*);

  bool VisitVarDecl(clang::VarDecl*);
  bool VisitFunctionDecl(clang::FunctionDecl*);
  bool VisitFieldDecl(clang::FieldDecl*);
  bool VisitTypedefDecl(clang::TypedefDecl*);
  bool VisitTypeAliasDecl(clang::TypeAliasDecl*);
  bool TraverseDecl(clang::Decl*);

private:
  bool TraverseArrayType(clang::ArrayType*);

private:
  void substituteTypedefNameDecl(clang::TypedefNameDecl*);
  void substituteVarDeclType(clang::DeclaratorDecl*);
  void substituteReturnType(clang::FunctionDecl*);

private:
  struct TypeInfo_t {
    void reset();
    bool isInited() const;
    void shouldVisitTypes(bool flag);
    bool shouldVisitTypes();
    void addConst();
    TypeInfo_t& operator<<(const std::string& Str);
    std::string getTypeAsString() const;

  private:
    void init();

  private:
    std::stringstream Buffer_;
    bool shouldVisitTypes_{false};
    bool isQualPrev_{false};
    bool isInited_{false};
  };

private:
  void endTraversingSubtree();

private:
  bool needSubstitution_{false};
  TypeInfo_t Type_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
