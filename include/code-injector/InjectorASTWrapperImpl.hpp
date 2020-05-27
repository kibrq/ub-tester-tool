#pragma once

#include "UBUtility.h"
#include <optional>
#include <vector>

namespace ub_tester::code_injector::wrapper {

using namespace util;

namespace {

inline std::string getArgAsString(std::string String,
                                  const clang::ASTContext*) {
  return String;
}

inline std::string getArgAsString(const clang::SourceRange& Range,
                                  const clang::ASTContext* Context) {
  return getRangeAsString(Range, Context);
}

inline std::string getArgAsString(const clang::Expr* Expr,
                                  const clang::ASTContext* Context) {
  return getExprAsString(Expr, Context);
}

template <typename T>
void generateArgumentsForSubstitutionHelper(const clang::ASTContext* Context,
                                            SubstArgs& Vec, const T& Arg) {
  Vec.push_back(getArgAsString(Arg, Context));
}

template <typename T>
void generateArgumentsForSubstitutionHelper(const clang::ASTContext* Context,
                                            SubstArgs& Vec,
                                            const std::optional<T>& Arg) {
  if (Arg.has_value())
    generateArgumentsForSubstitutionHelper(Context, Vec, Arg.value());
}

template <typename T, typename U>
void generateArgumentsForSubstitutionHelper(const clang::ASTContext* Context,
                                            SubstArgs& Vec, const T& Arg1,
                                            const U& Arg2) {
  generateArgumentsForSubstitutionHelper(Context, Vec, Arg1);
  generateArgumentsForSubstitutionHelper(Context, Vec, Arg2);
}

template <typename T, typename U, typename... ArgTypes>
void generateArgumentsForSubstitutionHelper(const clang::ASTContext* Context,
                                            SubstArgs& Vec, const T& Arg1,
                                            const U& Arg2, ArgTypes... Args) {
  generateArgumentsForSubstitutionHelper(Context, Vec, Arg1);
  generateArgumentsForSubstitutionHelper(Context, Vec, Arg2, Args...);
}

inline SubstArgs
generateArgumentsForSubstitution(const clang::ASTContext* Context) {
  return {};
}

template <typename T>
SubstArgs generateArgumentsForSubstitution(const clang::ASTContext* Context,
                                           const T& Arg) {
  SubstArgs Res;
  generateArgumentsForSubstitutionHelper(Context, Res, Arg);
  return Res;
}

template <typename T, typename U, typename... ArgTypes>
SubstArgs generateArgumentsForSubstitution(const clang::ASTContext* Context,
                                           const T& Arg1, const U& Arg2,
                                           ArgTypes... Args) {
  SubstArgs Res;
  generateArgumentsForSubstitutionHelper(Context, Res, Arg1, Arg2, Args...);
  return Res;
}

} // namespace

template <typename... ExprTypes>
SubstArgs createSubstArgs(const clang::ASTContext* Context,
                          ExprTypes... Exprs) {
  return generateArgumentsForSubstitution(Context, Exprs...);
}

} // namespace ub_tester::code_injector::wrapper
