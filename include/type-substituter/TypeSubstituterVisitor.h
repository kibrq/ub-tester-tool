#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <sstream>

namespace ub_tester {
class TypeSubstituterVisitor
    : public clang::RecursiveASTVisitor<TypeSubstituterVisitor> {
public:
  explicit TypeSubstituterVisitor(clang::ASTContext*);

  bool TraverseVariableArrayType(clang::VariableArrayType*);
  bool TraverseDependentSizedArrayType(clang::DependentSizedArrayType*);
  bool TraverseIncompleteArrayType(clang::IncompleteArrayType*);
  bool TraverseConstantArrayType(clang::ConstantArrayType*);
  bool TraverseRValueReferenceType(clang::RValueReferenceType*);
  bool TraverseLValueReferenceType(clang::LValueReferenceType*);
  bool TraversePointerType(clang::PointerType*);
  bool TraverseBuiltinType(clang::BuiltinType*);
  bool TraverseRecordType(clang::RecordType*);
  bool TraverseEnumType(clang::EnumType*);
  bool TraverseTemplateTypeParmType(clang::TemplateTypeParmType*);
  bool TraverseTemplateSpecializationType(clang::TemplateSpecializationType*);
  bool TraverseType(clang::QualType);

  bool TraverseDeclStmt(clang::DeclStmt*, DataRecursionQueue* = nullptr);

  bool VisitParmVarDecl(clang::ParmVarDecl*);
  bool VisitVarDecl(clang::VarDecl*);
  bool VisitFunctionDecl(clang::FunctionDecl*);
  bool VisitFieldDecl(clang::FieldDecl*);
  bool VisitTypedefNameDecl(clang::TypedefNameDecl*);
  // bool VisitTypedefDecl(clang::TypedefDecl*);
  // bool VisitTypeAliasDecl(clang::TypeAliasDecl*);
  bool TraverseDecl(clang::Decl*);

private:
  bool HelperTraverseArrayType(clang::ArrayType*);
  bool HelperVisitDeclaratorDecl(clang::DeclaratorDecl*);

private:
  void substituteTypeOfTypedef(clang::TypedefNameDecl*);
  void substituteTypeOfVariable(clang::DeclaratorDecl*);
  void substituteTypeOfReturn(clang::FunctionDecl*);

private:
  struct TypeInfo_t {
    void reset();
    bool isInited() const;
    void shouldVisitTypes(bool flag);
    bool shouldVisitTypes();
    void addQuals(clang::Qualifiers, const clang::PrintingPolicy&);
    TypeInfo_t& operator<<(const std::string& Str);
    std::string getTypeAsString() const;

  private:
    void init();

  private:
    std::stringstream Buffer_;
    std::optional<std::string> PrevQual_;
    bool shouldVisitTypes_{false};
    bool isInited_{false};
  };

private:
  bool isDeclStmtParent_{false};
  bool needSubstitution_{false};
  TypeInfo_t Type_;
  clang::ASTContext* Context_;
};

} // namespace ub_tester
