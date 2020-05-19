#include "type-substituter/TypeSubstituterVisitor.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "type-substituter/SafeTypesView.h"

#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace ub_tester {
using namespace types_view;

// TODO templates

TypeSubstituterVisitor::TypeSubstituterVisitor(ASTContext* Context)
    : Context_{Context} {}

void TypeSubstituterVisitor::TypeInfo_t::init() { isInited_ = true; }

void TypeSubstituterVisitor::TypeInfo_t::reset() {
  Buffer_.str("");
  isInited_ = shouldVisitTypes_ = false;
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

void TypeSubstituterVisitor::TypeInfo_t::addQuals(Qualifiers Quals,
                                                  const PrintingPolicy& PP) {
  Buffer_ << Quals.getAsString(PP);
  if (!Quals.isEmptyWhenPrinted(PP)) {
    Buffer_ << " ";
  }
}

bool TypeSubstituterVisitor::TypeInfo_t::isInited() const { return isInited_; }

void TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes(bool flag) {
  shouldVisitTypes_ = flag;
}

bool TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes() {
  return shouldVisitTypes_;
}

bool TypeSubstituterVisitor::HelperTraverseArrayType(ArrayType* T) {

  Type_ << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(
      T->getElementType());
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseVariableArrayType(VariableArrayType* T) {
  return HelperTraverseArrayType(T);
}

bool TypeSubstituterVisitor::TraverseDependentSizedArrayType(
    DependentSizedArrayType* T) {
  Type_ << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDependentSizedArrayType(
      T);
  if (T->getSizeExpr()) {
    Type_ << ", " << getExprAsString(T->getSizeExpr(), Context_);
  }
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseIncompleteArrayType(
    IncompleteArrayType* T) {
  return HelperTraverseArrayType(T);
}

bool TypeSubstituterVisitor::TraverseConstantArrayType(ConstantArrayType* T) {
  Type_ << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayType(T);
  Type_ << ", " << T->getSize().toString(10, false) << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseRValueReferenceType(
    RValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseRValueReferenceType(T);
  if (Type_.isInited())
    Type_ << "&&";
  return true;
}

bool TypeSubstituterVisitor::TraverseLValueReferenceType(
    LValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseLValueReferenceType(T);
  if (Type_.isInited())
    Type_ << "&";
  return true;
}

bool TypeSubstituterVisitor::TraversePointerType(PointerType* T) {
  Type_ << SafePointer << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraversePointerType(T);
  Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseBuiltinType(BuiltinType* T) {
  bool firstInit = !Type_.isInited();
  if (firstInit)
    Type_ << SafeBuiltinVar << "<";
  Type_ << T->getName(PrintingPolicy{Context_->getLangOpts()}).str();
  if (firstInit)
    Type_ << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseEnumType(EnumType* T) {
  if (Type_.isInited()) {
    Type_ << T->getDecl()->getQualifiedNameAsString();
  }
  return true;
}

bool TypeSubstituterVisitor::TraverseRecordType(RecordType* T) {
  if (Type_.isInited()) {
    Type_ << T->getDecl()->getQualifiedNameAsString();
  }
  return true;
}

bool TypeSubstituterVisitor::TraverseTemplateTypeParmType(
    TemplateTypeParmType* T) {
  if (Type_.isInited()) {
    Type_ << T->desugar().getAsString();
  }
  return true;
}

bool TypeSubstituterVisitor::TraverseTemplateSpecializationType(
    TemplateSpecializationType* T) {

  Type_ << T->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString()
        << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseTemplateName(
      T->getTemplateName());

  auto* Args = T->getArgs();
  unsigned NumArgs = T->getNumArgs();
  for (unsigned I = 0; I != NumArgs; ++I) {
    RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseTemplateArgument(
        Args[I]);
    if (I != NumArgs - 1)
      Type_ << ", ";
  }
  Type_ << ">";

  return true;
}

bool TypeSubstituterVisitor::TraverseType(QualType T) {
  if (!Type_.shouldVisitTypes())
    return true;
  Type_.addQuals(T.getLocalQualifiers(),
                 PrintingPolicy{Context_->getLangOpts()});
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(T);
  return true;
}

bool TypeSubstituterVisitor::TraverseDeclStmt(DeclStmt* DS,
                                              DataRecursionQueue* Queue) {
  isDeclStmtParent_ = true;
  needSubstitution_ = true;
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDeclStmt(DS, Queue);
  isDeclStmtParent_ = false;
  return true;
}

// FIXME static NEQ hasGlobalStorage()

void TypeSubstituterVisitor::substituteTypeOfVariable(DeclaratorDecl* DDecl) {
  if (!Type_.isInited())
    return;

  std::stringstream NewDeclaration;
  if (VarDecl* VDecl = dyn_cast<VarDecl>(DDecl)) {
    if (VDecl->isInline()) {
      NewDeclaration << "inline ";
    }
    if (VDecl->isConstexpr()) {
      NewDeclaration << "constexpr ";
    }
    if (VDecl->hasGlobalStorage()) {
      NewDeclaration << "static ";
    }
  }

  NewDeclaration << Type_.getTypeAsString() << " " << DDecl->getNameAsString();

  InjectorASTWrapper::getInstance().substitute(
      {DDecl->getBeginLoc(), getNameLastLoc(DDecl, Context_)},
      NewDeclaration.str(), Context_);
  Type_.reset();
}

void TypeSubstituterVisitor::substituteTypeOfReturn(FunctionDecl* FDecl) {
  if (!Type_.isInited())
    return;

  InjectorASTWrapper::getInstance().substitute(
      FDecl->getReturnTypeSourceRange(), Type_.getTypeAsString(), Context_);
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

bool TypeSubstituterVisitor::VisitFunctionDecl(FunctionDecl* FDecl) {
  if (!Context_->getSourceManager().isWrittenInMainFile(FDecl->getBeginLoc()))
    return true;

  if (FDecl && !FDecl->getReturnType().getTypePtr()->isVoidType() &&
      !FDecl->isMain()) {
    Type_.shouldVisitTypes(true);
    TraverseType(FDecl->getReturnType());
    substituteTypeOfReturn(FDecl);
  }
  return true;
}

bool TypeSubstituterVisitor::HelperVisitDeclaratorDecl(DeclaratorDecl* D) {
  if (!Context_->getSourceManager().isWrittenInMainFile(D->getBeginLoc()))
    return true;
  Type_.shouldVisitTypes(true);
  TraverseType(D->getType());
  substituteTypeOfVariable(D);
  return true;
} // namespace

bool TypeSubstituterVisitor::VisitParmVarDecl(ParmVarDecl* PVD) {
  return HelperVisitDeclaratorDecl(PVD);
}

bool TypeSubstituterVisitor::VisitVarDecl(VarDecl* VDecl) {
  if (!isa<ParmVarDecl>(VDecl) &&
      (!isDeclStmtParent_ || (isDeclStmtParent_ && needSubstitution_))) {
    HelperVisitDeclaratorDecl(VDecl);
    needSubstitution_ = false;
  }
  return true;
}

bool TypeSubstituterVisitor::VisitFieldDecl(FieldDecl* FD) {
  return HelperVisitDeclaratorDecl(FD);
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
