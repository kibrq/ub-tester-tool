#include "clang/AST/ASTConsumer.h"

#include "index-out-of-bounds/CArrayHandler.h"
#include "index-out-of-bounds/PointerSubstituter.h"

#include <string>

namespace ub_tester {

class IndexOutOfBoundsConsumer : public clang::ASTConsumer {
public:
  explicit IndexOutOfBoundsConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  CArrayHandler ArrayHandler_;
  PointerSubstituter pointer_sub_;
};

} // namespace ub_tester
