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

bool PointerHandler::TraverseDecl(Decl* D) { return true; }

} // namespace ub_tester
