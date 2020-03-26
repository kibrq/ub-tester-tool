#include "arithmetic-overflow/FindArithmeticOverflowVisitor.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class FindArithmeticOverflowConsumer : public clang::ASTConsumer {
public:
  explicit FindArithmeticOverflowConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindArithmeticOverflowVisitor Visitor;
};

} // namespace ub_tester
