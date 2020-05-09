#include "type-substituter/TypeSubstituterVisitor.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "type-substituter/SafeTypesView.h"

#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace ub_tester {
using namespace types_view;

// TODO templates
// TODO constexpr

TypeSubstituterVisitor::TypeSubstituterVisitor(ASTContext* Context)
    : Context_{Context} {}

void TypeSubstituterVisitor::TypeInfo_t::init() {
  if (!Name_.has_value()) {
    Name_.emplace();
  }
  FirstInit_ = FirstInit_ == 0 ? 1 : -1;
}

void TypeSubstituterVisitor::TypeInfo_t::reset() {
  Name_ = std::nullopt;
  FirstInit_ = 0;
  isQualPrev = shouldVisitTypes_ = false;
}

std::stringstream& TypeSubstituterVisitor::TypeInfo_t::getName() {
  init();
  isQualPrev = false;
  return *Name_;
}

const std::stringstream& TypeSubstituterVisitor::TypeInfo_t::getName() const {
  assert(Name_.has_value());
  return *Name_;
}

void TypeSubstituterVisitor::TypeInfo_t::addConst() {
  init();
  if (!isQualPrev) {
    *Name_ << "const ";
    isQualPrev = true;
  }
}

bool TypeSubstituterVisitor::TypeInfo_t::isFirstInit() const {
  return FirstInit_ == 1;
}

bool TypeSubstituterVisitor::TypeInfo_t::isInited() const {
  return FirstInit_ != 0;
}

void TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes(bool flag) {
  shouldVisitTypes_ = flag;
}

bool TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes() {
  return shouldVisitTypes_;
}

bool TypeSubstituterVisitor::TraverseArrayType(ArrayType* T) {
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(
      T->getElementType());
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseVariableArrayType(VariableArrayType* T) {
  return TraverseArrayType(T);
}

bool TypeSubstituterVisitor::TraverseDepedentSizedArrayType(
    DependentSizedArrayType* T) {
  return TraverseArrayType(T);
}

bool TypeSubstituterVisitor::TraverseIncompleteArrayType(
    IncompleteArrayType* T) {
  return TraverseArrayType(T);
}

bool TypeSubstituterVisitor::TraverseConstantArrayType(ConstantArrayType* T) {
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayType(T);
  Type_.getName() << ", " << T->getSize().toString(10, false) << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseRValueReferenceType(
    RValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseRValueReferenceType(T);
  Type_.getName() << "&&";
  return true;
}

bool TypeSubstituterVisitor::TraverseLValueReferenceType(
    LValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseLValueReferenceType(T);
  Type_.getName() << "&";
  return true;
}

bool TypeSubstituterVisitor::TraversePointerType(PointerType* T) {
  Type_.getName() << SafePointer << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraversePointerType(T);
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseBuiltinType(BuiltinType* T) {
  Type_.getName() << SafeBuiltinVar << "<";
  Type_.getName() << T->getName(PrintingPolicy{Context_->getLangOpts()}).str();
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseType(QualType T) {
  if (!Type_.shouldVisitTypes())
    return true;
  if (T.isConstQualified()) {
    Type_.addConst();
  }
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(T);
  return true;
}

void TypeSubstituterVisitor::substituteVarDeclType(DeclaratorDecl* VDecl) {
  std::string NewString{
      (dyn_cast<VarDecl>(VDecl) && dyn_cast<VarDecl>(VDecl)->hasGlobalStorage())
          ? "static "
          : ""};
  Type_.getName() << " " << VDecl->getNameAsString();
  NewString += Type_.getName().str();

  ASTFrontendInjector::getInstance().substitute(
      Context_, {VDecl->getBeginLoc(), getNameLastLoc(VDecl, Context_)},
      std::move(NewString));
}

void TypeSubstituterVisitor::substituteReturnType(FunctionDecl* FDecl) {
  ASTFrontendInjector::getInstance().substitute(
      Context_, FDecl->getReturnTypeSourceRange(), Type_.getName().str());
}

bool TypeSubstituterVisitor::VisitDeclStmt(DeclStmt* DS) {
  needSubstitution_ = true;
  return true;
}

bool TypeSubstituterVisitor::TraverseDecl(Decl* D) {
  DeclaratorDecl* VDecl = nullptr;
  const Type* T = nullptr;
  if (D && (VDecl = dyn_cast<DeclaratorDecl>(D))) {
    if (not Context_->getSourceManager().isWrittenInMainFile(
            VDecl->getBeginLoc()))
      return true;
    T = VDecl->getType().getTypePtrOrNull();
    Type_.shouldVisitTypes(T->isBuiltinType() || T->isPointerType() ||
                           T->isArrayType() || T->isReferenceType());
  }

  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDecl(D);
  if (VDecl) {
    if (Type_.shouldVisitTypes() && needSubstitution_) {
      TraverseType(VDecl->getType());
      substituteVarDeclType(VDecl);
    } else if (FunctionDecl* FDecl = dyn_cast<FunctionDecl>(VDecl);
               FDecl && !FDecl->getReturnType().getTypePtr()->isVoidType() &&
               !FDecl->isMain()) {
      Type_.shouldVisitTypes(true);
      TraverseType(FDecl->getReturnType());
      substituteReturnType(FDecl);
    }
    Type_.reset();
    needSubstitution_ = false;
  }
  return true;
}

} // namespace ub_tester
