#include "clang/AST/ASTConsumer.h"

#include "index-out-of-bounds/CArraySubstituter.h"
#include "index-out-of-bounds/PointerSubstituter.h"

#include <string>

namespace ub_tester {

class SubstituterConsumer : public clang::ASTConsumer {
public:
  explicit SubstituterConsumer(clang::ASTContext* Context);
  virtual void HandleTranslationUnit(clang::ASTContext& Context);

private:
  CArraySubstituter array_sub_;
  PointerSubstituter pointer_sub_;
};

} // namespace ub_tester
