#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

#include <optional>
#include <sstream>

namespace ub_tester {
class TypeSubstituterVisitor
    : public clang::RecursiveASTVisitor<TypeSubstituterVisitor> {
public:
  explicit TypeSubstituterVisitor(clang::ASTContext*);

  bool TraverseBuiltinTypeLoc(clang::BuiltinTypeLoc);
  bool TraverseConstantArrayTypeLoc(clang::ConstantArrayTypeLoc);
  bool TraversePointerTypeLoc(clang::PointerTypeLoc);

  bool TraverseDecl(clang::Decl*);

private:
  struct TypeInfo_t {
    TypeInfo_t() { reset(); }
    void reset() {
      Name_ = std::nullopt;
      shouldVisitTypes_ = FirstInit_ = 0;
    }
    void init() {
      FirstInit_ = FirstInit_ == 0 ? 1 : -1;
      if (!Name_.has_value()) {
        Name_.emplace();
      }
    }
    std::stringstream& getName() { return *Name_; }
    const std::stringstream& getName() const { return *Name_; }
    bool isFirstInit() const { return FirstInit_ == 1; }
    bool isInited() const { return FirstInit_ != 0; }
    void shouldVisitTypes(bool flag) { shouldVisitTypes_ = flag; }
    bool shouldVisitTypes() { return shouldVisitTypes_; }

  private:
    std::optional<std::stringstream> Name_;
    bool shouldVisitTypes_{false};
    int FirstInit_ = 0;
  };

private:
  TypeInfo_t Type_;
  clang::ASTContext* Context_;
};
} // namespace ub_tester
