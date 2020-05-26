#pragma once

#include "UBUtility.h"

#include <optional>
#include <vector>

namespace ub_tester {

using code_injector::SubstArgs;

namespace {

inline std::string getArgAsString(const std::string& Arg,
                                  const clang::ASTContext*) {
  return Arg;
}

inline std::string getArgAsString(const clang::SourceRange& Range,
                                  const clang::ASTContext* Context) {
  return getRangeAsString(Range, Context);
}

inline std::string getArgAsString(const clang::Expr* Ex,
                                  const clang::ASTContext* Context) {
  return getExprAsString(Ex, Context);
}

inline std::string getArgAsString(int a, const clang::ASTContext* Context) {
  return std::to_string(a);
}

template <typename T>
inline void generateArgumentsForSubstitution_(const clang::ASTContext* Context,
                                              SubstArgs& Vec, const T& Arg) {
  Vec.push_back(getArgAsString(Arg, Context));
}

template <typename T>
inline void generateArgumentsForSubstitution_(const clang::ASTContext* Context,
                                              SubstArgs& Vec,
                                              const std::optional<T>& Arg) {
  if (Arg.has_value()) {
    generateArgumentsForSubstitution_(Context, Vec, Arg.value());
  }
}

template <typename T, typename U>
inline void generateArgumentsForSubstitution_(const clang::ASTContext* Context,
                                              SubstArgs& Vec, const T& Arg1,
                                              const U& Arg2) {
  generateArgumentsForSubstitution_(Context, Vec, Arg1);
  generateArgumentsForSubstitution_(Context, Vec, Arg2);
}

template <typename T, typename U, typename... W>
inline void generateArgumentsForSubstitution_(const clang::ASTContext* Context,
                                              SubstArgs& Vec, const T& Arg1,
                                              const U& Arg2, W... ws) {
  generateArgumentsForSubstitution_(Context, Vec, Arg1);
  generateArgumentsForSubstitution_(Context, Vec, Arg2, ws...);
}

inline SubstArgs
generateArgumentsForSubstitution(const clang::ASTContext* Context) {
  return {};
}

template <typename T>
inline SubstArgs
generateArgumentsForSubstitution(const clang::ASTContext* Context,
                                 const T& Arg) {
  SubstArgs Result;
  generateArgumentsForSubstitution_(Context, Result, Arg);
  return Result;
}

template <typename T, typename U, typename... W>
inline SubstArgs
generateArgumentsForSubstitution(const clang::ASTContext* Context,
                                 const T& Arg1, const U& Arg2, W... ws) {
  SubstArgs Result;
  generateArgumentsForSubstitution_(Context, Result, Arg1, Arg2, ws...);
  return Result;
}
} // namespace

template <typename... Expr>
SubstArgs createSubstArgs(const clang::ASTContext* Context, Expr... Exprs) {
  return generateArgumentsForSubstitution(Context, Exprs...);
}

} // namespace ub_tester
