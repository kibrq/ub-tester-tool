#include "pointers/PointerHandler.h"
#include "UBUtility.h"

#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace ub_tester {
void PointerHandler::PointerInfo_t::reset() {
  shouldVisitDeclRefExpr_ = shouldVisitNodes_ = false;
  Name_ = Type_ = Size_ = {};
}

PointerHandler::PointerHandler(ASTContext* Context) : Context_{Context} {
  Pointer_.reset();
}

bool PointerHandler::TraverseDecl(Decl* D) {
  const Type* T = nullptr;
  ValueDecl* VDecl = nullptr;
  if (D && (VDecl = dyn_cast<ValueDecl>(D))) {
    if (not Context_->getSourceManager().isWrittenInMainFile(
            VDecl->getBeginLoc()))
      return true;

    T = isa<ParmVarDecl>(VDecl)
            ? dyn_cast<ParmVarDecl>(VDecl)->getOriginalType().getTypePtrOrNull()
            : VDecl->getType().getTypePtrOrNull();
    Array_.shouldVisitNodes_ = T && T->isArrayType();
  }
  RecursiveASTVisitor<CArrayHandler>::TraverseDecl(D);
  if (T && T->isArrayType()) {
    Array_.Name_ = VDecl->getName().str();
    executeSubstitutionOfArrayDecl(VDecl);
  }
  Pointer_.reset();
  return true;
}

} // namespace ub_tester
