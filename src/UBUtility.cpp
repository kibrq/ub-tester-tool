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

QualType getLowestLevelPointeeType(QualType QT) {
  if (auto* PT = llvm::dyn_cast<PointerType>(QT)) {
    if (isa<PointerType>(PT->getPointeeType().getTypePtrOrNull())) {
      return getLowestLevelPointeeType(PT->getPointeeType());
    }
    return PT->getPointeeType().getUnqualifiedType();
  }
  return QT;
}
} // namespace ub_tester
