#include "index-out-of-bounds/PointerSubstituter.h"
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

PointerSubstituter::PointerSubstituter(ASTContext* Context)
    : Context(Context) {}

bool isPointer(const Type* t) {
  return t && t->isPointerType() && !t->isFunctionPointerType();
}

bool PointerSubstituter::VisitFunctionDecl(FunctionDecl* fd) {
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

bool PointerSubstituter::VisitPointerType(PointerType* pt) {
  if (pointer_.should_visit_nodes_) {
    pointer_.type_ = pt->getPointeeType().getAsString();
  }
  return true;
}

bool PointerSubstituter::VisitCXXNewExpr(CXXNewExpr* cne) {
  // TODO
  return true;
}

bool PointerSubstituter::VisitCallExpr(CallExpr* ce) {
  if (pointer_.should_visit_nodes_) {
    FunctionDecl* decl = ce->getDirectCallee();
    if (Context->getSourceManager().isInExternCSystemHeader(
            decl->getBeginLoc())) {
      std::string function_name = decl->getNameInfo().getAsString();
      if (allocation_functions_.find(function_name) !=
          allocation_functions_.end()) {
        pointer_.size_ =
            allocation_functions_[function_name](ce, pointer_.type_, Context);
      }
    }
  }
  return true;
}

bool PointerSubstituter::TraverseVarDecl(VarDecl* vd) {
  if (Context->getSourceManager().isInMainFile(vd->getBeginLoc())) {
    auto type = vd->getType().getTypePtrOrNull();
    pointer_.should_visit_nodes_ = isPointer(type);
    if (pointer_.should_visit_nodes_) {
      RecursiveASTVisitor<PointerSubstituter>::TraverseVarDecl(vd);
      pointer_.name_ = vd->getNameAsString();
      printf(
          "UBSafePointer<%s> %s\n%s.setSize(%s)\n", pointer_.type_.c_str(),
          pointer_.name_.c_str(), pointer_.name_.c_str(),
          pointer_.size_.c_str());
    }
    pointer_.should_visit_nodes_ = false;
  }
  return true;
}

bool PointerSubstituter::VisitDeclRefExpr(DeclRefExpr* ref_expr) {
  if (should_visit_decl_ref_expr_) {

    auto type = ref_expr->getDecl()->getType().getTypePtrOrNull();
    if (type && type->isPointerType()) {
      pointer_.name_ = ref_expr->getDecl()->getNameAsString();
      pointer_.should_visit_nodes_ = true;
    }
  }
  return true;
}

bool PointerSubstituter::VisitBinaryOperator(BinaryOperator* bo) {
  if (Context->getSourceManager().isInMainFile(bo->getBeginLoc())) {
    should_visit_decl_ref_expr_ = true;
    RecursiveASTVisitor<PointerSubstituter>::TraverseStmt(bo->getLHS());
    should_visit_decl_ref_expr_ = false;
    if (pointer_.should_visit_nodes_) {
      RecursiveASTVisitor<PointerSubstituter>::TraverseStmt(bo->getRHS());
      printf(
          "%s.setSize(%s)\n", pointer_.name_.c_str(), pointer_.size_.c_str());
      pointer_.should_visit_nodes_ = false;
    }
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
