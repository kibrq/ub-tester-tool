#include "arithmetic-overflow/FindArithmeticUBVisitor.h"
#include "clang/AST/ASTConsumer.h"

namespace ub_tester {

class FindArithmeticUBConsumer : public clang::ASTConsumer {
public:
  explicit FindArithmeticUBConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  FindArithmeticUBVisitor Visitor;
};

} // namespace ub_tester
