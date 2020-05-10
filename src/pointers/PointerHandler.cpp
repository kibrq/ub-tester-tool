#include "pointers/PointerHandler.h"
#include "UBUtility.h"
#include "code-injector/ASTFrontendInjector.h"

#include "clang/Basic/SourceManager.h"

#include <unordered_map>

// TODO add support of more allocation funcs

using namespace clang;

namespace ub_tester {

PointerHandler::PointerInfo_t::PointerInfo_t(bool shouldVisitNodes)
    : shouldVisitNodes_{shouldVisitNodes} {}

bool PointerHandler::shouldVisitNodes() {
  return !Pointers_.empty() && Pointers_.back().shouldVisitNodes_;
}

void PointerHandler::reset() {
  if (!Pointers_.empty())
    Pointers_.pop_back();
}

PointerHandler::PointerInfo_t& PointerHandler::backPointer() {
  return Pointers_.back();
}

PointerHandler::PointerHandler(ASTContext* Context) : Context_{Context} {}

namespace {

using SizeCalculator = std::string (*)(CallExpr*, const std::string&,
                                       ASTContext* Context);

std::string MallocSize(CallExpr* CE, const std::string& PointeeType,
                       ASTContext* Context) {
  std::stringstream Res;
  Res << getExprAsString(CE->getArg(0), Context) << "/"
      << "sizeof(" << PointeeType << ")";
  return Res.str();
}

std::unordered_map<std::string, SizeCalculator> SizeCalculators = {
    {"malloc", &MallocSize}};

std::optional<SizeCalculator>
getSizeCalculationFunc(const std::string& FunctionName) {
  if (SizeCalculators.find(FunctionName) != SizeCalculators.end())
    return SizeCalculators[FunctionName];
  return {};
}

} // namespace

bool PointerHandler::VisitCallExpr(CallExpr* CE) {
  if (shouldVisitNodes() && CE->getDirectCallee()) {
    auto Calculator = getSizeCalculationFunc(
        CE->getDirectCallee()->getNameInfo().getAsString());
    if (Calculator) {
      backPointer().hasSize_ = true;
      backPointer().Size_ << Calculator.value()(CE, backPointer().PointeeType_,
                                                Context_);
    }
  }
  return true;
}

std::pair<std::string, std::string> PointerHandler::getCtorFormats() {
  std::string SourceFormat = backPointer().Init_.has_value() ? "#@" : "";
  std::stringstream OutputFormat;
  OutputFormat << "(" << (backPointer().Init_.has_value() ? "@" : "")
               << (backPointer().hasSize_ ? ", " : "")
               << (backPointer().hasSize_ > 0 ? Pointers_.back().Size_.str()
                                              : "")
               << ")";
  return {SourceFormat, OutputFormat.str()};
}

void PointerHandler::executeSubstitutionOfPointerCtor(VarDecl* VDecl) {
  SourceLocation Loc = getAfterNameLoc(VDecl, Context_);
  auto Formats = getCtorFormats();
  ASTFrontendInjector::getInstance().substitute(
      Context_, Loc, Formats.first, Formats.second, Pointers_.back().Init_);
}

#define NOT_IN_MAINFILE(Context, Node)                                         \
  if (!Context->getSourceManager().isWrittenInMainFile(Node->getBeginLoc()))   \
    return true;

bool PointerHandler::TraverseVarDecl(clang::VarDecl* VDecl) {

  NOT_IN_MAINFILE(Context_, VDecl);

  Pointers_.emplace_back(VDecl->getType().getTypePtr()->isPointerType());
  if (shouldVisitNodes()) {
    backPointer().PointeeType_ =
        dyn_cast<PointerType>(VDecl->getType().getTypePtr())
            ->getPointeeType()
            .getAsString();
  }

  RecursiveASTVisitor<PointerHandler>::TraverseVarDecl(VDecl);
  if (shouldVisitNodes()) {
    if (VDecl->hasInit()) {
      backPointer().Init_ = getExprAsString(VDecl->getInit(), Context_);
    }
    executeSubstitutionOfPointerCtor(VDecl);
  }
  reset();
  return true;
}

std::pair<std::string, std::string> PointerHandler::getAssignFormats() {
  std::string SourceFormat = "@";
  std::stringstream OutputFormat;
  OutputFormat << "(@).setSize(" << backPointer().Size_.str() << ")";
  return {SourceFormat, OutputFormat.str()};
}

void PointerHandler::executeSubstitutionOfPointerAssignment(
    BinaryOperator* BO) {
  SourceLocation Loc = BO->getBeginLoc();
  auto Formats = getAssignFormats();
  ASTFrontendInjector::getInstance().substitute(Context_, Loc, Formats.first,
                                                Formats.second, BO->getLHS());
}

bool PointerHandler::TraverseBinAssign(BinaryOperator* BO,
                                       DataRecursionQueue* Queue) {
  NOT_IN_MAINFILE(Context_, BO);
  if (BO->getLHS()->getType().getTypePtr()->isPointerType()) {
    Pointers_.emplace_back(true);
    backPointer().PointeeType_ =
        dyn_cast<PointerType>(BO->getLHS()->getType().getTypePtr())
            ->getPointeeType()
            .getAsString();
  }

  RecursiveASTVisitor<PointerHandler>::TraverseStmt(BO->getLHS(), Queue);
  RecursiveASTVisitor<PointerHandler>::TraverseStmt(BO->getRHS());
  if (shouldVisitNodes()) {
    executeSubstitutionOfPointerAssignment(BO);
  }
  reset();
  return true;
}

} // namespace ub_tester
