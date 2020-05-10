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

void TypeSubstituterVisitor::TypeInfo_t::init() { isInited_ = true; }

void TypeSubstituterVisitor::TypeInfo_t::reset() {
  Buffer_.str("");
  isInited_ = isQualPrev_ = shouldVisitTypes_ = false;
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

// FIXME
void TypeSubstituterVisitor::TypeInfo_t::addConst() {
  init();
  if (!isQualPrev_) {
    Buffer_ << "const ";
    isQualPrev_ = true;
  }
}

bool TypeSubstituterVisitor::TypeInfo_t::isInited() const { return isInited_; }

void TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes(bool flag) {
  shouldVisitTypes_ = flag;
}

bool TypeSubstituterVisitor::TypeInfo_t::shouldVisitTypes() {
  return shouldVisitTypes_;
}

bool TypeSubstituterVisitor::TraverseArrayType(ArrayType* T) {
  Type_ << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(
      T->getElementType());
  Type_ << ">";
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
  Type_ << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayType(T);
  Type_ << ", " << T->getSize().toString(10, false) << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseRValueReferenceType(
    RValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseRValueReferenceType(T);
  Type_ << "&&";
  return true;
}

bool TypeSubstituterVisitor::TraverseLValueReferenceType(
    LValueReferenceType* T) {
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseLValueReferenceType(T);
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
  if (T.isConstQualified()) {
    Type_.addConst();
  }
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(T);
  return true;
}

bool TypeSubstituterVisitor::VisitDeclStmt(DeclStmt* DS) {
  needSubstitution_ = true;
  return true;
}

namespace {
enum class TargetKind {
  FunctionDecl,
  FieldDecl,
  VarDecl,
  TypedefDecl,
  TypeAliasDecl,
  None
};

std::vector<TargetKind> TargetStack;

bool isTargetFunction(FunctionDecl* FDecl) {
  return FDecl && !FDecl->getReturnType().getTypePtr()->isVoidType() &&
         !FDecl->isMain();
}
} // namespace

bool TypeSubstituterVisitor::VisitFunctionDecl(FunctionDecl*) {
  TargetStack.back() = TargetKind::FunctionDecl;
  return true;
}

bool TypeSubstituterVisitor::VisitVarDecl(VarDecl*) {
  TargetStack.back() = TargetKind::VarDecl;
  return true;
}

bool TypeSubstituterVisitor::VisitFieldDecl(FieldDecl*) {
  TargetStack.back() = TargetKind::FieldDecl;
  return true;
}

bool TypeSubstituterVisitor::VisitTypedefDecl(TypedefDecl*) {
  TargetStack.back() = TargetKind::TypedefDecl;
  return true;
}

bool TypeSubstituterVisitor::VisitTypeAliasDecl(TypeAliasDecl*) {
  TargetStack.back() = TargetKind::TypeAliasDecl;
  return true;
}

void TypeSubstituterVisitor::substituteVarDeclType(DeclaratorDecl* VDecl) {
  if (!Type_.isInited())
    return;

  Type_ << " " << VDecl->getNameAsString();

  ASTFrontendInjector::getInstance().substitute(
      Context_, {VDecl->getBeginLoc(), getNameLastLoc(VDecl, Context_)},
      Type_.getTypeAsString());
}

void TypeSubstituterVisitor::substituteReturnType(FunctionDecl* FDecl) {
  if (Type_.isInited())
    return;

  ASTFrontendInjector::getInstance().substitute(
      Context_, FDecl->getReturnTypeSourceRange(), Type_.getTypeAsString());
}

void TypeSubstituterVisitor::substituteTypedefNameDecl(TypedefNameDecl* TDecl) {
  if (!Type_.isInited())
    return;

  ASTFrontendInjector::getInstance().substitute(
      Context_, TDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange(),
      Type_.getTypeAsString());
}

void TypeSubstituterVisitor::endTraversingSubtree() {
  Type_.reset();
  needSubstitution_ = false;
  TargetStack.pop_back();
}

bool TypeSubstituterVisitor::TraverseDecl(Decl* D) {

  TargetStack.push_back(TargetKind::None);
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDecl(D);

  if (D && TargetStack.back() != TargetKind::None &&
      Context_->getSourceManager().isWrittenInMainFile(D->getBeginLoc())) {

    switch (TargetStack.back()) {
    case TargetKind::FieldDecl:
      needSubstitution_ = true;
    case TargetKind::VarDecl: {
      DeclaratorDecl* VDecl = dyn_cast<DeclaratorDecl>(D);
      assert(VDecl);
      if (needSubstitution_) {
        Type_.shouldVisitTypes(true);
        TraverseType(VDecl->getType());
        substituteVarDeclType(VDecl);
      }
      break;
    }
    case TargetKind::FunctionDecl: {
      if (FunctionDecl* FDecl = dyn_cast<FunctionDecl>(D);
          isTargetFunction(FDecl)) {
        Type_.shouldVisitTypes(true);
        TraverseType(FDecl->getReturnType());
        substituteReturnType(FDecl);
      }
      return true;
    }
    case TargetKind::TypedefDecl:
    case TargetKind::TypeAliasDecl: {
      TypedefNameDecl* TDecl = dyn_cast<TypedefNameDecl>(D);
      assert(TDecl);
      Type_.shouldVisitTypes(true);
      TraverseType(TDecl->getTypeSourceInfo()->getType());
      substituteTypedefNameDecl(TDecl);
      break;
    }
    case TargetKind::None:
      assert(false);
      break;
    }
  }
  endTraversingSubtree();
  return true;
}
} // namespace ub_tester
