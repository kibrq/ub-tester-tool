#include "UBUtility.h"
#include "clang/Lex/Lexer.h"

#include <sstream>

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
  if (!FullLocation.isValid())
    return "invalid location";
  res << FullLocation.getSpellingLineNumber() << ":"
      << FullLocation.getSpellingColumnNumber();
  return res.str();
}

} // namespace ub_tester
