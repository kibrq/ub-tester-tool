#include "clang/AST/TypeLoc.h"
#include "clang/Lex/Lexer.h"

#include "UBUtility.h"

#include <cassert>
#include <sstream>

using namespace clang;

namespace ub_tester {

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

SourceLocation getNameLastLoc(SourceLocation BeginLoc, std::string_view VarName,
                              const ASTContext* Context) {
  const auto& SM = Context->getSourceManager();
  const auto& LO = Context->getLangOpts();
  while (true) {
    auto Tok = Lexer::findNextToken(BeginLoc, SM, LO);
    assert(Tok.hasValue());
    if (Tok->is(tok::raw_identifier)) {
      if (Tok->getRawIdentifier().str().compare(VarName) == 0) {
        return Tok->getLastLoc();
      }
    }
    if (Tok->isOneOf(tok::semi, tok::equal, tok::comma, tok::r_paren)) {
      return BeginLoc;
    }
    BeginLoc = Tok->getLocation();
  }
}
} // namespace

// FIXME comma

SourceLocation getNameLastLoc(const DeclaratorDecl* Decl,
                              const ASTContext* Context) {
  return getNameLastLoc(Decl->getTypeSourceInfo()->getTypeLoc().getEndLoc(),
                        Decl->getNameAsString(), Context);
}

// FIXME delete whitespace

SourceLocation getAfterNameLoc(const DeclaratorDecl* Decl,
                               const ASTContext* Context) {
  return Lexer::findNextToken(getNameLastLoc(Decl, Context),
                              Context->getSourceManager(),
                              Context->getLangOpts())
      ->getLocation();
}
} // namespace ub_tester
