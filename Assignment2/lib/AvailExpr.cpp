/**
 * @file Available Expression Dataflow Analysis
 */
#include <iterator>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>

#include <dfa/Framework.h>
#include <dfa/MeetOp.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

#include "Expression.h"

using namespace dfa;
using namespace llvm;

namespace {

using AvailExprFrameworkBase =
    Framework<Expression, bool, Direction::kForward, Intersect>;

class AvailExpr final : public AvailExprFrameworkBase, public FunctionPass {
private:
  virtual void initializeDomainFromInst(const Instruction &Inst) override {
    if (isa<BinaryOperator>(Inst)){
      if (position(*dyn_cast<BinaryOperator>(&Inst)) == -1) {
        Domain.emplace_back(Expression(Inst));
      }
    }
  }

  virtual bool transferFunc(const Instruction &Inst, const DomainVal_t &IBV,
                            DomainVal_t &OBV) override {
    /**
     * @todo(cscd70) Please complete the definition of the transfer function.
     */
    DomainVal_t TmpDV = IBV;


    // const Value *ValueInst = dyn_cast<Value>(&Inst);
    // for (auto &Expr: Domain){
    //   if (ValueInst == Expr.LHS || ValueInst == Expr.RHS) {
    //     // kill set 
    //     TmpDV[position(Expr)] = false;
    //   }
    // }

    if (isa<BinaryOperator>(Inst)){
      TmpDV[position(*dyn_cast<BinaryOperator>(&Inst))] = true;
    }

    bool isChange = TmpDV != OBV;
    OBV = TmpDV;
    return isChange;

  }

public:
  static char ID;

  AvailExpr() : AvailExprFrameworkBase(), FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
  bool runOnFunction(Function &F) override {
    AvailExprFrameworkBase::runOnFunction(F);
    return false;
  }
};

char AvailExpr::ID = 0;
RegisterPass<AvailExpr> X("avail-expr", "Available Expression");

} // anonymous namespace
