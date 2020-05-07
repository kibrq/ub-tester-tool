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
  init();
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
  if (T.isConstQualified()) {
    Type_.getName() << "const ";
  }
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseType(T);
  return true;
}

namespace {

SourceLocation getEndSubstitutionLoc(SourceLocation BeginLoc,
                                     std::string_view VarName,
                                     ASTContext* Context) {
  auto& SM = Context->getSourceManager();
  const auto& LO = Context->getLangOpts();
  while (true) {
    auto Tok = Lexer::findNextToken(BeginLoc, SM, LO);
    assert(Tok.hasValue());
    if (Tok->is(tok::raw_identifier)) {
      if (Tok->getRawIdentifier().str().compare(VarName) == 0) {
        return Tok->getEndLoc();
      }
    }
    if (Tok->isOneOf(tok::semi, tok::equal, tok::comma, tok::r_paren)) {
      return BeginLoc;
    }
    BeginLoc = Tok->getLocation();
  }
}

} // namespace

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
  if (Type_.shouldVisitTypes() && VDecl) {
    TraverseType(VDecl->getType());
  } else {
    RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseDecl(D);
  }
  if (!VDecl) {
    return true;
  }
  if (Type_.isInited()) {

    std::string NewString{(dyn_cast<VarDecl>(VDecl) &&
                           dyn_cast<VarDecl>(VDecl)->hasGlobalStorage())
                              ? "static "
                              : ""};
    Type_.getName() << " " << VDecl->getNameAsString();
    NewString += Type_.getName().str();

    SourceRange Range{VDecl->getBeginLoc(),
                      VDecl->getTypeSourceInfo()->getTypeLoc().getEndLoc()};
    getEndSubstitutionLoc(Range.getEnd(), VDecl->getNameAsString(), Context_)
        .dump(Context_->getSourceManager());

    Range.setEnd(getEndSubstitutionLoc(Range.getEnd(), VDecl->getNameAsString(),
                                       Context_));

    ASTFrontendInjector::getInstance().substitute(Context_, std::move(Range),
                                                  std::move(NewString));
  }
  Type_.reset();
  if (FunctionDecl* FDecl = dyn_cast<FunctionDecl>(VDecl);
      FDecl && !FDecl->getReturnType().getTypePtr()->isVoidType()) {
    TraverseType(FDecl->getReturnType());
    ASTFrontendInjector::getInstance().substitute(
        Context_, FDecl->getReturnTypeSourceRange(), Type_.getName().str());
  }
  return true;
}

} // namespace ub_tester
