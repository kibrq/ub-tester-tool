#include "type-substituter/TypeSubstituterVisitor.h"
#include "code-injector/ASTFrontendInjector.h"

#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace ub_tester {

TypeSubstituterVisitor::TypeSubstituterVisitor(ASTContext* Context)
    : Context_{Context} {}

bool TypeSubstituterVisitor::TraverseBuiltinTypeLoc(BuiltinTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  if (Type_.isFirstInit())
    Type_.getName() << "safeVar<";
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
  Type_.getName() << "safeArray<";
  RecursiveASTVisitor<TypeSubstituterVisitor>::TraverseConstantArrayTypeLoc(T);
  Type_.getName() << ">";
  return true;
}

bool TypeSubstituterVisitor::TraversePointerTypeLoc(PointerTypeLoc T) {
  if (!Type_.shouldVisitTypes()) {
    return true;
  }
  Type_.init();
  Type_.getName() << "safeArray<";
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

    Type_.getName() << " " << VDecl->getNameAsString();
    llvm::outs() << Type_.getName().str() << "\n";
    ASTFrontendInjector::getInstance().substitute(
        Context_, VDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange(),
        Type_.getName().str());
  }
  Type_.reset();
  return true;
}

} // namespace ub_tester
