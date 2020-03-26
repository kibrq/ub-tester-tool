#include "UBUtility.h"
#include "clang/Lex/Lexer.h"

using namespace clang;

namespace ub_tester {

std::string getExprAsString(const Expr* ex, const ASTContext* Context) {
  return Lexer::getSourceText(
             CharSourceRange::getTokenRange(ex->getSourceRange()),
             Context->getSourceManager(), Context->getLangOpts())
      .str();
}

std::string getExprLineNCol(const Expr* Expression, const ASTContext* Context) {
  FullSourceLoc FullLocation = Context->getFullLoc(Expression->getBeginLoc());
  std::stringstream res;
  res << FullLocation.getSpellingLineNumber() << ":"
      << FullLocation.getSpellingColumnNumber();
  if (FullLocation.isValid())
    return res.str();
  return "invalid location";
}

} // namespace ub_tester
