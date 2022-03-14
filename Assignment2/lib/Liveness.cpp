/**
 * @file Liveness Dataflow Analysis
 */
#include "Variable.h"
#include "dfa/Framework.h"
#include "dfa/MeetOp.h"
#include <llvm/IR/Argument.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace dfa;

namespace {

using LivenessFrameworkBase =
    Framework<Variable, bool, dfa::Direction::kBackward, Union>;

class Liveness final : public LivenessFrameworkBase, public FunctionPass {
private:
  virtual void initializeDomainFromInst(const Instruction &Inst) override {
    for (const auto &Op : Inst.operands()) {
      if (isa<Value>(Op) || isa<Argument>(Op)) {
        if (position(Variable(Op)) == -1) {
          Domain.push_back(Variable(Op));
        }
      }
    }
  }

  virtual bool transferFunc(const Instruction &Inst, const DomainVal_t &IV,
                            DomainVal_t &OV) override {

    DomainVal_t tmp = IV;

    const Value *ValunInst = dyn_cast<Value>(&Inst);
    for (auto &Var : Domain) {
      if (Var.V == ValunInst) {
        tmp[position(Var)] = false;
      }
    }

    for (auto &Op : Inst.operands()) {
      // gen
      if (isa<Value>(Op) || isa<Argument>(Op)) {
        tmp[position(Variable(Op))] = true;
      }
    }

    bool isChange = OV != tmp;
    OV = tmp;
    return isChange;
  }

public:
  static char ID;

  Liveness() : LivenessFrameworkBase(), FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  virtual bool runOnFunction(Function &F) override {
    LivenessFrameworkBase::runOnFunction(F);
    return false;
  }
};

char Liveness::ID = 0;
RegisterPass<Liveness> X("liveness", "Liveness");

} // anonymous namespace
