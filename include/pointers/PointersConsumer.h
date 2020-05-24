#include "clang/AST/ASTConsumer.h"

#include "pointers/PointerHandler.h"

namespace ub_tester {

class PointersConsumer : public clang::ASTConsumer {
public:
  explicit PointersConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  PointerHandler PointerHandler_;
};

} // namespace ub_tester
