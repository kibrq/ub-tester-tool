#include "UBUtility.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace clang;

namespace ub_tester::util {

std::string getExprAsString(const Expr* Ex, const ASTContext* Context) {
  return getRangeAsString(Ex->getSourceRange(), Context);
}

std::string getRangeAsString(const SourceRange& Range,
                             const ASTContext* Context) {
  return Lexer::getSourceText(CharSourceRange::getTokenRange(Range),
                              Context->getSourceManager(),
                              Context->getLangOpts())
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

QualType getLowestLevelPointeeType(QualType QT) {
  if (auto* PT = llvm::dyn_cast<PointerType>(QT)) {
    if (isa<PointerType>(PT->getPointeeType().getTypePtrOrNull())) {
      return getLowestLevelPointeeType(PT->getPointeeType());
    }
    return PT->getPointeeType().getUnqualifiedType();
  }
  return QT;
}

namespace {

SourceLocation getBeforeNameLoc(SourceLocation BeginLoc, SourceLocation EndLoc,
                                std::string_view VarName,
                                const SourceManager& SM,
                                const LangOptions& LO) {
  while (SM.isBeforeInTranslationUnit(BeginLoc, EndLoc)) {
    auto Tok = Lexer::findNextToken(BeginLoc, SM, LO);
    assert(Tok.hasValue());
    if (Tok->isAnyIdentifier() &&
        Tok->getRawIdentifier().str().compare(VarName) == 0) {
      return BeginLoc;
    }
    BeginLoc = Tok->getLocation();
  }
  return EndLoc;
}

} // namespace

SourceLocation getNameLastLoc(const DeclaratorDecl* Decl,
                              const ASTContext* Context) {
  SourceLocation BeginLoc, EndLoc;
  const auto& SM = Context->getSourceManager();
  const auto& LO = Context->getLangOpts();
  SourceLocation Res = getBeforeNameLoc(
      BeginLoc = Decl->getTypeSourceInfo()->getTypeLoc().getEndLoc(),
      EndLoc = Decl->getEndLoc(), Decl->getNameAsString(), SM, LO);
  if (SM.isBeforeInTranslationUnit(Res, EndLoc)) {
    return Lexer::findNextToken(Res, SM, LO)->getLastLoc();
  } else {
    return BeginLoc;
  }
}

SourceLocation getAfterNameLoc(const DeclaratorDecl* Decl,
                               const ASTContext* Context) {
  SourceLocation BeginLoc, EndLoc;
  const auto& SM = Context->getSourceManager();
  const auto& LO = Context->getLangOpts();
  SourceLocation Res = getBeforeNameLoc(
      BeginLoc = Decl->getTypeSourceInfo()->getTypeLoc().getEndLoc(),
      EndLoc = Decl->getEndLoc(), Decl->getNameAsString(), SM, LO);
  if (SM.isBeforeInTranslationUnit(Res, EndLoc)) {
    return Lexer::findNextToken(Res, SM, LO)->getEndLoc();
  } else {
    return BeginLoc.getLocWithOffset(1);
  }
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
  return FuncsWithAvailCode.find(getFuncNameWithArgsAsString(FuncDecl)) !=
         FuncsWithAvailCode.end();
}

void setHasFuncAvailCode(const clang::FunctionDecl* FuncDecl) {
  FuncsWithAvailCode.insert(getFuncNameWithArgsAsString(FuncDecl));
}

GetFuncCodeAvailVisitor::GetFuncCodeAvailVisitor(clang::ASTContext* Context)
    : Context(Context) {}

bool GetFuncCodeAvailVisitor::VisitFunctionDecl(FunctionDecl* FuncDecl) {
  if (!Context->getSourceManager().isInMainFile(FuncDecl->getBeginLoc()))
    return true;

  if (FuncDecl->doesThisDeclarationHaveABody())
    func_code_avail::setHasFuncAvailCode(FuncDecl);
  return true;
}

UtilityConsumer::UtilityConsumer(clang::ASTContext* Context)
    : FuncCodeAvailVisitor(Context) {}

void UtilityConsumer::HandleTranslationUnit(clang::ASTContext& Context) {
  FuncCodeAvailVisitor.TraverseDecl(Context.getTranslationUnitDecl());
}

} // namespace func_code_avail
} // namespace ub_tester::util
