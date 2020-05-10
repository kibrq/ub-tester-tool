#include "type-substituter/TypeSubstituterVisitor.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
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

namespace {
enum class TargetKind {
  FunctionDecl,
  ParmVarDecl,
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

bool TypeSubstituterVisitor::VisitParmVarDecl(ParmVarDecl*) {
  TargetStack.back() = TargetKind::ParmVarDecl;
  return true;
}

bool TypeSubstituterVisitor::VisitVarDecl(VarDecl*) {
  if (TargetStack.back() == TargetKind::None)
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

void TypeSubstituterVisitor::substituteVarDeclType(DeclaratorDecl* DDecl) {
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

  ASTFrontendInjector::getInstance().substitute(
      Context_, {DDecl->getBeginLoc(), getNameLastLoc(DDecl, Context_)},
      NewDeclaration.str());
}

void TypeSubstituterVisitor::substituteReturnType(FunctionDecl* FDecl) {
  if (!Type_.isInited())
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
  assert(!TargetStack.empty());
  if (D && TargetStack.back() != TargetKind::None &&
      Context_->getSourceManager().isWrittenInMainFile(D->getBeginLoc())) {

    switch (TargetStack.back()) {
    case TargetKind::ParmVarDecl:
    case TargetKind::FieldDecl:
    case TargetKind::VarDecl: {
      DeclaratorDecl* DDecl = dyn_cast<DeclaratorDecl>(D);
      assert(DDecl);
      if (!isDeclStmtParent_ || (isDeclStmtParent_ && needSubstitution_)) {
        Type_.shouldVisitTypes(true);
        TraverseType(DDecl->getType());
        substituteVarDeclType(DDecl);
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
      break;
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
