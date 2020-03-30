#include "clang/Lex/Lexer.h"

#include "UBUtility.h"

#include <cassert>

using namespace clang;

namespace ub_tester {

std::string getExprAsString(const Expr* ex, const ASTContext* Context) {
  return Lexer::getSourceText(
             CharSourceRange::getTokenRange(ex->getSourceRange()),
             Context->getSourceManager(), Context->getLangOpts())
      .str();
}

std::string getLowestLevelPointeeType(const Type* T) {
  if (auto* PT = llvm::dyn_cast<PointerType>(T)) {
    if (llvm::isa<PointerType>(PT->getPointeeType().getTypePtrOrNull())) {
      return getLowestLevelPointeeType(PT);
    }
    return PT->getPointeeType().getUnqualifiedType().getAsString();
  }
  return "";
}
} // namespace ub_tester
