#include "pointers/PointerHandler.h"
#include "UBUtility.h"
#include "code-injector/InjectorASTWrapper.h"
#include "pointers/PointersAssertsView.h"
#include "clang/Basic/SourceManager.h"
#include <unordered_map>

// TODO add support of more allocation funcs

using namespace clang;
using namespace ub_tester::code_injector;
using namespace ub_tester::code_injector::wrapper;

namespace ub_tester {

PointerVisitor::PointerInfo_t::PointerInfo_t(bool ShouldVisitNodes)
    : ShouldVisitNodes_{ShouldVisitNodes} {}

PointerVisitor::PointerInfo_t& PointerVisitor::backPointer() {
  return Pointers_.back();
}

bool PointerVisitor::shouldVisitNodes() {
  return !Pointers_.empty() && backPointer().ShouldVisitNodes_;
}

void PointerVisitor::reset() {
  if (!Pointers_.empty())
    Pointers_.pop_back();
}

PointerVisitor::PointerVisitor(ASTContext* Context) : Context_{Context} {}

namespace {

using SizeCalculatorType = std::string (*)(CallExpr*, const std::string&,
                                           ASTContext* Context);

std::string calculateMallocSize(CallExpr* CE, const std::string& PointeeType,
                                ASTContext* Context) {
  std::stringstream Res;
  Res << getExprAsString(CE->getArg(0), Context) << "/"
      << "sizeof(" << PointeeType << ")";
  return Res.str();
}

std::unordered_map<std::string, SizeCalculatorType> SizeCalculators = {
    {"malloc", &calculateMallocSize}};

std::optional<SizeCalculatorType>
getSizeCalculationFunc(const std::string& FunctionName) {
  if (SizeCalculators.find(FunctionName) != SizeCalculators.end())
    return SizeCalculators[FunctionName];
  return {};
}

} // namespace

bool PointerVisitor::VisitCallExpr(CallExpr* CE) {
  if (shouldVisitNodes() && CE->getDirectCallee()) {
    auto Calculator = getSizeCalculationFunc(
        CE->getDirectCallee()->getNameInfo().getAsString());
    if (Calculator) {
      backPointer().HasSize_ = true;
      backPointer().Size_ << Calculator.value()(CE, backPointer().PointeeType_,
                                                Context_);
    }
  }
  return true;
}

bool PointerVisitor::VisitCXXNewExpr(CXXNewExpr* CNE) {
  if (shouldVisitNodes()) {
    backPointer().HasSize_ = true;
    if (!CNE->isArray())
      backPointer().Size_ << "1";
    else
      backPointer().Size_ << getExprAsString(CNE->getArraySize().getValue(),
                                             Context_);
  }
  return true;
}

std::pair<std::string, std::string> PointerVisitor::getCtorFormats() {
  std::string SourceFormat = backPointer().Init_.has_value() ? "#@" : "";
  std::stringstream OutputFormat;
  OutputFormat << "(" << (backPointer().Init_.has_value() ? "@" : "")
               << (backPointer().HasSize_ ? ", " : "")
               << (backPointer().HasSize_ > 0 ? backPointer().Size_.str() : "")
               << ")";
  return {SourceFormat, OutputFormat.str()};
}

void PointerVisitor::executeSubstitutionOfPointerCtor(VarDecl* VDecl) {
  SourceLocation Loc = getAfterNameLoc(VDecl, Context_);
  auto Formats = getCtorFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(Pointers_.back().Init_)
      .apply();
}

bool PointerVisitor::TraverseVarDecl(clang::VarDecl* VDecl) {
  if (!Context_->getSourceManager().isWrittenInMainFile(VDecl->getBeginLoc()))
    return true;

  Pointers_.emplace_back(VDecl->getType().getTypePtr()->isPointerType());
  if (shouldVisitNodes())
    backPointer().PointeeType_ =
        dyn_cast<PointerType>(VDecl->getType().getTypePtr())
            ->getPointeeType()
            .getAsString();

  RecursiveASTVisitor<PointerVisitor>::TraverseVarDecl(VDecl);
  if (shouldVisitNodes()) {
    if (VDecl->hasInit())
      backPointer().Init_ = getExprAsString(VDecl->getInit(), Context_);
    executeSubstitutionOfPointerCtor(VDecl);
  }
  reset();
  return true;
}

std::pair<std::string, std::string> PointerVisitor::getAssignFormats() {
  std::string SourceFormat = "@";
  std::stringstream OutputFormat;
  OutputFormat << "(@).setSize(" << backPointer().Size_.str() << ")";
  return {SourceFormat, OutputFormat.str()};
}

void PointerVisitor::executeSubstitutionOfPointerAssignment(
    BinaryOperator* Binop) {
  SourceLocation Loc = Binop->getBeginLoc();
  auto Formats = getAssignFormats();
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setFormats(Formats.first, Formats.second)
      .setArguments(Binop->getLHS())
      .apply();
}

bool PointerVisitor::TraverseBinAssign(BinaryOperator* Binop,
                                       DataRecursionQueue* Queue) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Binop->getBeginLoc()))
    return true;

  if (Binop->getLHS()->getType().getTypePtr()->isPointerType()) {
    Pointers_.emplace_back(true);
    backPointer().PointeeType_ =
        dyn_cast<PointerType>(Binop->getLHS()->getType().getTypePtr())
            ->getPointeeType()
            .getAsString();
  }

  RecursiveASTVisitor<PointerVisitor>::TraverseStmt(Binop->getLHS());
  RecursiveASTVisitor<PointerVisitor>::TraverseStmt(Binop->getRHS());
  if (shouldVisitNodes())
    executeSubstitutionOfPointerAssignment(Binop);
  reset();
  return true;
}

void PointerVisitor::executeSubstitutionOfStarOperator(UnaryOperator* Unop) {
  SourceLocation Loc = Unop->getBeginLoc();
  std::string SourceFormat = "*@";
  std::string OutputFormat =
      ptr::names_to_inject::getAssertStarOperatorAsString("@");
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setPrior(SubstPriorityKind::Deep)
      .setFormats(SourceFormat, OutputFormat)
      .setArguments(Unop->getSubExpr())
      .apply();
}

bool PointerVisitor::VisitUnaryOperator(UnaryOperator* Unop) {
  if (!Context_->getSourceManager().isWrittenInMainFile(Unop->getBeginLoc()))
    return true;
  if (Unop->getOpcode() == UnaryOperator::Opcode::UO_Deref &&
      Unop->getSubExpr()->getType()->isPointerType())
    executeSubstitutionOfStarOperator(Unop);
  return true;
}

void PointerVisitor::executeSubstitutionOfMemberExpr(MemberExpr* MembExpr) {
  SourceLocation Loc = MembExpr->getBeginLoc();
  std::string SourceFormat = "@#@";
  std::string OutputFormat =
      ptr::names_to_inject::getAssertMemberExprAsString("@", "@");
  SubstitutionASTWrapper(Context_)
      .setLoc(Loc)
      .setPrior(SubstPriorityKind::Deep)
      .setFormats(SourceFormat, OutputFormat)
      .setArguments(MembExpr->getBase(), SourceRange{MembExpr->getMemberLoc(),
                                                     MembExpr->getEndLoc()})
      .apply();
}

bool PointerVisitor::VisitMemberExpr(MemberExpr* MembExpr) {
  if (!Context_->getSourceManager().isWrittenInMainFile(
          MembExpr->getBeginLoc()))
    return true;
  if (MembExpr->isArrow())
    executeSubstitutionOfMemberExpr(MembExpr);
  return true;
}

} // namespace ub_tester
