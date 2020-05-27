#include "type-substituter/TypeSubstituterVisitor.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "type-substituter/SafeTypesView.h"
#include "clang/Basic/SourceManager.h"

// TODO templates

using namespace clang;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

namespace ub_tester {

using namespace typenames_to_inject;

TypeSubstituterVisitor::TypeSubstituterVisitor(ASTContext* Context)
    : Context_{Context} {}

void TypeSubstituterVisitor::TypeInfo_t::init() { IsInited_ = true; }

void TypeSubstituterVisitor::TypeInfo_t::reset() {
  Buffer_.str("");
  IsInited_ = ShouldVisitTypes_ = false;
}

TypeSubstituterVisitor::TypeInfo_t&
TypeSubstituterVisitor::TypeInfo_t::operator<<(const std::string& Str) {
  init();
  Buffer_ << Str;
  return *this;
}

std::string TypeSubstituterVisitor::TypeInfo_t::getTypeAsString() const {
  return Buffer_.str();
}

void TypeSubstituterVisitor::TypeInfo_t::addQuals(
    Qualifiers Quals, const PrintingPolicy& PPolicy) {
  Buffer_ << Quals.getAsString(PPolicy);
  if (!Quals.isEmptyWhenPrinted(PPolicy))
    Buffer_ << " ";
}

bool TypeSubstituterVisitor::TypeInfo_t::isInited() const { return IsInited_; }

void TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes(bool Value) {
  ShouldVisitTypes_ = Value;
}

bool TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes() {
  return ShouldVisitTypes_;
}

bool TypeSubstituterVisitor::TraverseArrayTypeHelper(ArrayType* ArrType) {
  Type_ << SafeArrayName << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(
      ArrType->getElementType());
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseVariableArrayType(
    VariableArrayType* VarArrType) {
  return TraverseArrayTypeHelper(VarArrType);
}

bool TypeSubstituterVisitor::TraverseDependentSizedArrayType(
    DependentSizedArrayType* DepSizedArrType) {
  Type_ << SafeArrayName << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDependentSizedArrayType(
      DepSizedArrType);
  if (DepSizedArrType->getSizeExpr())
    Type_ << ", " << getExprAsString(DepSizedArrType->getSizeExpr(), Context_);
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseIncompleteArrayType(
    IncompleteArrayType* IncArrType) {
  return TraverseArrayTypeHelper(IncArrType);
}

bool TypeSubstituterVisitor::TraverseConstantArrayType(
    ConstantArrayType* ConstArrType) {
  Type_ << SafeArrayName << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayType(
      ConstArrType);
  Type_ << ", " << ConstArrType->getSize().toString(10, false) << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseRValueReferenceType(
    RValueReferenceType* RValRefType) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseRValueReferenceType(
      RValRefType);
  if (Type_.isInited())
    Type_ << "&&";
  return true;
}

bool TypeSubstituterVisitor::TraverseLValueReferenceType(
    LValueReferenceType* LValRefType) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseLValueReferenceType(
      LValRefType);
  if (Type_.isInited())
    Type_ << "&";
  return true;
}

bool TypeSubstituterVisitor::TraversePointerType(PointerType* PtrType) {
  Type_ << SafePointerName << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraversePointerType(PtrType);
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseBuiltinType(BuiltinType* BType) {
  bool FirstInit = !Type_.isInited();
  if (FirstInit)
    Type_ << SafeBuiltinVarName << "<";
  Type_ << BType->getName(PrintingPolicy{Context_->getLangOpts()}).str();
  if (FirstInit)
    Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseEnumType(EnumType* EType) {
  if (Type_.isInited())
    Type_ << EType->getDecl()->getQualifiedNameAsString();
  return true;
}

bool TypeSubstituterVisitor::TraverseRecordType(RecordType* RecType) {
  if (Type_.isInited())
    Type_ << RecType->getDecl()->getQualifiedNameAsString();
  return true;
}

bool TypeSubstituterVisitor::TraverseTypedefType(TypedefType* TType) {
  if (Type_.isInited())
    Type_ << TType->getDecl()->getQualifiedNameAsString();
  return true;
}

bool TypeSubstituterVisitor::TraverseTemplateTypeParmType(
    TemplateTypeParmType* TemplParmType) {
  if (Type_.isInited())
    Type_ << TemplParmType->desugar().getAsString();
  return true;
}

bool TypeSubstituterVisitor::TraverseTemplateSpecializationType(
    TemplateSpecializationType* TemplSpecType) {
  Type_ << TemplSpecType->getTemplateName()
               .getAsTemplateDecl()
               ->getQualifiedNameAsString()
        << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseTemplateName(
      TemplSpecType->getTemplateName());

  auto* Args = TemplSpecType->getArgs();
  size_t NumArgs = TemplSpecType->getNumArgs();
  for (size_t I = 0; I != NumArgs; ++I) {
    RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseTemplateArgument(
        Args[I]);
    if (I != NumArgs - 1)
      Type_ << ", ";
  }
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseType(QualType QType) {
  if (!Type_.shouldVisitTypes())
    return true;
  Type_.addQuals(QType.getLocalQualifiers(),
                 PrintingPolicy{Context_->getLangOpts()});
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(QType);
  return true;
}

bool TypeSubstituterVisitor::TraverseDeclStmt(DeclStmt* DStmt,
                                              DataRecursionQueue* Queue) {
  IsDeclStmtParent_ = NeedSubstitution_ = true;
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDeclStmt(DStmt, Queue);
  IsDeclStmtParent_ = false;
  return true;
}

// FIXME static NEQ hasGlobalStorage()

void TypeSubstituterVisitor::substituteTypeOfVariable(
    DeclaratorDecl* DeclarDecl) {
  if (!Type_.isInited())
    return;
  std::stringstream NewDeclaration;
  if (VarDecl* VDecl = dyn_cast<VarDecl>(DeclarDecl)) {
    if (VDecl->isInline())
      NewDeclaration << "inline ";
    if (VDecl->isConstexpr())
      NewDeclaration << "constexpr ";
    if (VDecl->hasGlobalStorage())
      NewDeclaration << "static ";
  }
  NewDeclaration << Type_.getTypeAsString() << " "
                 << DeclarDecl->getNameAsString();
  InjectorASTWrapper::getInstance().substitute(
      {DeclarDecl->getBeginLoc(), getNameLastLoc(DeclarDecl, Context_)},
      NewDeclaration.str(), Context_);
  Type_.reset();
}

void TypeSubstituterVisitor::substituteTypeOfReturn(FunctionDecl* FuncDecl) {
  if (!Type_.isInited())
    return;
  InjectorASTWrapper::getInstance().substitute(
      FuncDecl->getReturnTypeSourceRange(), Type_.getTypeAsString(), Context_);
  Type_.reset();
}

void TypeSubstituterVisitor::substituteTypeOfTypedef(TypedefNameDecl* TDecl) {
  if (!Type_.isInited())
    return;
  InjectorASTWrapper::getInstance().substitute(
      TDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange(),
      Type_.getTypeAsString(), Context_);
  Type_.reset();
}

bool TypeSubstituterVisitor::VisitFunctionDecl(FunctionDecl* FuncDecl) {
  const auto& SrcManager = Context_->getSourceManager();
  if (!SrcManager.isWrittenInMainFile(FuncDecl->getBeginLoc()) ||
      SrcManager.isMacroBodyExpansion(FuncDecl->getBeginLoc()))
    return true;

  if (FuncDecl && !FuncDecl->getReturnType().getTypePtr()->isVoidType() &&
      !FuncDecl->isMain()) {
    Type_.shouldVisitTypes(true);
    TraverseType(FuncDecl->getReturnType());
    substituteTypeOfReturn(FuncDecl);
  }
  return true;
}

bool TypeSubstituterVisitor::VisitDeclaratorDeclHelper(
    DeclaratorDecl* DeclarDecl) {
  const auto& SrcManager = Context_->getSourceManager();
  if (!SrcManager.isWrittenInMainFile(DeclarDecl->getBeginLoc()))
    return true;
  Type_.shouldVisitTypes(true);
  TraverseType(DeclarDecl->getType());
  substituteTypeOfVariable(DeclarDecl);
  return true;
}

bool TypeSubstituterVisitor::VisitParmVarDecl(ParmVarDecl* PVDecl) {
  return VisitDeclaratorDeclHelper(PVDecl);
}

bool TypeSubstituterVisitor::VisitVarDecl(VarDecl* VDecl) {
  if (!isa<ParmVarDecl>(VDecl) &&
      (!IsDeclStmtParent_ || (IsDeclStmtParent_ && NeedSubstitution_))) {
    VisitDeclaratorDeclHelper(VDecl);
    NeedSubstitution_ = false;
  }
  return true;
}

bool TypeSubstituterVisitor::VisitFieldDecl(FieldDecl* FDecl) {
  return VisitDeclaratorDeclHelper(FDecl);
}

bool TypeSubstituterVisitor::VisitTypedefNameDecl(TypedefNameDecl* TDecl) {
  if (!Context_->getSourceManager().isWrittenInMainFile(TDecl->getBeginLoc()))
    return true;
  Type_.shouldVisitTypes(true);
  TraverseType(TDecl->getTypeSourceInfo()->getType());
  substituteTypeOfTypedef(TDecl);
  return true;
}

} // namespace ub_tester
