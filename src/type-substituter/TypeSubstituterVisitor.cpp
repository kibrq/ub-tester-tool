#include "type-substituter/TypeSubstituterVisitor.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"
#include "type-substituter/SafeTypesView.h"

#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace ub_tester {
using namespace types_view;

// TODO add qualifieirs

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
  shouldVisitTypes_ = false;
}

std::stringstream& TypeSubstituterVisitor::TypeInfo_t::getName() {
  assert(Name_.has_value());
  return *Name_;
}

const std::stringstream& TypeSubstituterVisitor::TypeInfo_t::getName() const {
  assert(Name_.has_value());
  return *Name_;
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

bool TypeSubstituterVisitor::TraverseBuiltinTypeLoc(BuiltinTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  if (Type_.isFirstInit())
    Type_.getName() << SafeBuiltinVar << "<";
  Type_.getName() << T.getType().getAsString();
  if (Type_.isFirstInit())
    Type_.getName() << ">";

  return true;
}

bool TypeSubstituterVisitor::TraverseConstantArrayTypeLoc(
    ConstantArrayTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayTypeLoc(T);
  Type_.getName() << ", " << getExprAsString(T.getSizeExpr(), Context_) << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseVariableArrayTypeLoc(
    VariableArrayTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseVariableArrayTypeLoc(T);
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseDepedentSizedArrayTypeLoc(
    DependentSizedArrayTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<
      TypeSubstituterVisitor>::TraverseDependentSizedArrayTypeLoc(T);
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraverseIncompleteArrayTypeLoc(
    IncompleteArrayTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << SafeArray << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseIncompleteArrayTypeLoc(
      T);
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraversePointerTypeLoc(PointerTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << SafePointer << "<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraversePointerTypeLoc(T);
  Type_.getName() << ">";
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
  if (Type_.isInited()) {

    std::string NewString = (dyn_cast<VarDecl>(VDecl) &&
                             dyn_cast<VarDecl>(VDecl)->hasGlobalStorage())
                                ? "static "
                                : "";
    NewString += Type_.getName().str();

    SourceRange Range{VDecl->getBeginLoc(),
                      VDecl->getTypeSourceInfo()->getTypeLoc().getEndLoc()};

    llvm::outs() << VDecl->getQualifierLoc().hasQualifier();
    VDecl->getInnerLocStart().dump(Context_->getSourceManager());

    ASTFrontendInjector::getInstance().substitute(Context_, std::move(Range),
                                                  std::move(NewString));
  }
  Type_.reset();
  return true;
}

} // namespace ub_tester
