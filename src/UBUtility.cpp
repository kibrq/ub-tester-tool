#include "clang/Lex/Lexer.h"

#include "UBUtility.h"
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace clang;

namespace ub_tester {

std::string getExprAsString(const Expr* Ex, const ASTContext* Context) { return getRangeAsString(Ex->getSourceRange(), Context); }

std::string getRangeAsString(const SourceRange& Range, const ASTContext* Context) {
  return Lexer::getSourceText(CharSourceRange::getTokenRange(Range), Context->getSourceManager(), Context->getLangOpts()).str();
}

std::string getExprLineNCol(const Expr* Expression, const ASTContext* Context) {
  FullSourceLoc FullLocation = Context->getFullLoc(Expression->getBeginLoc());
  std::stringstream res;
  if (!FullLocation.isValid())
    return "invalid location";
  res << FullLocation.getSpellingLineNumber() << ":" << FullLocation.getSpellingColumnNumber();
  return res.str();
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

std::string getFuncNameWithArgsAsString(const clang::FunctionDecl* FuncDecl) {
  assert(FuncDecl);
  std::string Ans = FuncDecl->getNameAsString();
  for (ParmVarDecl* PVD : FuncDecl->parameters())
    Ans += PVD->getOriginalType().getAsString();
  std::replace(Ans.begin(), Ans.end(), ' ', '_');
  return Ans;
}

namespace func_code_avail {

std::unordered_set<std::string> FuncsWithAvailCode;

bool hasFuncAvailCode(const clang::FunctionDecl* FuncDecl) {
  if (!FuncDecl)
    return false;
  return FuncsWithAvailCode.find(getFuncNameWithArgsAsString(FuncDecl)) != FuncsWithAvailCode.end();
}

void setHasFuncAvailCode(const clang::FunctionDecl* FuncDecl) { FuncsWithAvailCode.insert(getFuncNameWithArgsAsString(FuncDecl)); }

} // namespace func_code_avail

} // namespace ub_tester
