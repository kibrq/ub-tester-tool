#include "index-out-of-bounds/PointerHandler.h"
#include "clang/Basic/SourceManager.h"
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace clang;

namespace {

using determine_size =
    std::string (*)(CallExpr*, const std::string& pointee_type, ASTContext*);

std::string malloc_determine_size(CallExpr*, const std::string&, ASTContext*);
std::string calloc_determine_size(CallExpr*, const std::string&, ASTContext*);
std::string realloc_determine_size(CallExpr*, const std::string&, ASTContext*);
std::string
alligned_alloc_determine_size(CallExpr*, const std::string&, ASTContext*);

std::unordered_map<std::string, determine_size> allocation_functions_ = {
    {"malloc", &malloc_determine_size},
    {"calloc", &calloc_determine_size},
    {"realloc", &realloc_determine_size},
    {"aligned_alloc", &alligned_alloc_determine_size}};

} // namespace

namespace ub_tester {

void PointerHandler::PointerInfo_t::reset() {
  shouldVisitNodes_ = shouldVisitDeclRefExpr_ = false;
}

PointerHandler::PointerHandler(ASTContext* Context) : Context(Context) {}

bool isPointer(const Type* t) {
  return t && t->isPointerType() && !t->isFunctionPointerType();
}

bool PointerHandler::VisitFunctionDecl(FunctionDecl* fd) {
  if (Context->getSourceManager().isInMainFile(fd->getBeginLoc())) {
    for (const auto& param : fd->parameters()) {
      auto type = param->getOriginalType().getTypePtrOrNull();
      if (isPointer(type)) {
        printf("Pointer in functionDecl\n");
      }
    }
    auto return_type = fd->getReturnType().getTypePtrOrNull();
    if (isPointer(return_type)) {
      printf("Pointer in return stmt\n");
    }
  }
  return true;
}

bool PointerHandler::VisitPointerType(PointerType* pt) {
  if (Pointer_.shouldVisitNodes_) {
    Pointer_.Type_ = pt->getPointeeType().getAsString();
  }
  return true;
}

bool PointerHandler::VisitCXXNewExpr(CXXNewExpr* cne) {
  // TODO
  return true;
}

bool PointerHandler::VisitCallExpr(CallExpr* ce) {
  if (Pointer_.shouldVisitNodes_) {
    FunctionDecl* decl = ce->getDirectCallee();
    if (Context->getSourceManager().isInExternCSystemHeader(
            decl->getBeginLoc())) {
      std::string function_name = decl->getNameInfo().getAsString();
      if (allocation_functions_.find(function_name) !=
          allocation_functions_.end()) {
        Pointer_.Size_ =
            allocation_functions_[function_name](ce, Pointer_.Type_, Context);
      }
    }
  }
  return true;
}

bool PointerHandler::TraverseVarDecl(VarDecl* vd) {
  if (Context->getSourceManager().isInMainFile(vd->getBeginLoc())) {
    auto type = vd->getType().getTypePtrOrNull();
    Pointer_.shouldVisitNodes_ = isPointer(type);
    if (Pointer_.shouldVisitNodes_) {
      RecursiveASTVisitor<PointerHandler>::TraverseVarDecl(vd);
      Pointer_.Name_ = vd->getNameAsString();
    }
    Pointer_.reset();
  }
  return true;
}

bool PointerHandler::VisitDeclRefExpr(DeclRefExpr* ref_expr) {
  if (Pointer_.shouldVisitNodes_) {
    auto type = ref_expr->getDecl()->getType().getTypePtrOrNull();
    if (type && type->isPointerType()) {
      Pointer_.Name_ = ref_expr->getDecl()->getNameAsString();
      Pointer_.shouldVisitNodes_ = true;
    }
  }
  return true;
}

bool PointerHandler::VisitBinaryOperator(BinaryOperator* bo) {
  if (Context->getSourceManager().isInMainFile(bo->getBeginLoc())) {
    Pointer_.shouldVisitDeclRefExpr_ = true;
    RecursiveASTVisitor<PointerHandler>::TraverseStmt(bo->getLHS());
    Pointer_.shouldVisitDeclRefExpr_ = false;
    if (Pointer_.shouldVisitNodes_) {
      RecursiveASTVisitor<PointerHandler>::TraverseStmt(bo->getRHS());
    }
    Pointer_.reset();
  }
  return true;
}

} // namespace ub_tester

namespace {

const std::string void_type = "void";

std::string malloc_determine_size(
    CallExpr* ce, const std::string& pointee_type, ASTContext* Context) {
  std::stringstream ss(ub_tester::getExprAsString(ce->getArgs()[0], Context));
  if (pointee_type.compare(void_type) == 0) {
    return ss.str();
  }
  ss << "/sizeof(" << pointee_type << ")";
  return ss.str();
}

std::string
calloc_determine_size(CallExpr* ce, const std::string&, ASTContext* Context) {
  return ub_tester::getExprAsString(ce->getArgs()[1], Context);
}

std::string realloc_determine_size(
    CallExpr* ce, const std::string& pointee_type, ASTContext* Context) {
  std::stringstream ss(ub_tester::getExprAsString(ce->getArgs()[1], Context));
  if (pointee_type.compare(void_type) == 0) {
    return ss.str();
  }
  ss << "/sizeof(" << pointee_type << ")";
  return ss.str();
}

std::string alligned_alloc_determine_size(
    CallExpr* ce, const std::string& pointee_type, ASTContext* Context) {
  std::stringstream ss(ub_tester::getExprAsString(ce->getArgs()[1], Context));
  if (pointee_type.compare(void_type) == 0) {
    return ss.str();
  }
  ss << "/sizeof(" << pointee_type << ")";
  return ss.str();
}

} // namespace
