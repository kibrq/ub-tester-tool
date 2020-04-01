#include "UBUtility.h"
#include "clang/Lex/Lexer.h"

using namespace clang;

namespace ub_tester {

std::string getExprAsString(const Expr* Ex, const ASTContext* Context) {
  return getRangeAsString(Ex->getSourceRange(), Context);
}

std::string
getRangeAsString(const SourceRange& Range, const ASTContext* Context) {
  return Lexer::getSourceText(
             CharSourceRange::getTokenRange(Range), Context->getSourceManager(),
             Context->getLangOpts())
      .str();
}

} // namespace ub_tester
