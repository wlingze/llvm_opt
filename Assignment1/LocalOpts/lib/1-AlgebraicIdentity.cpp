#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llvm;

namespace {

class AlgebraicIdentity final : public FunctionPass {
private:
  void deleteInstruction(std::vector<Instruction *> Insts) {
    for (auto &Inst : Insts) {
      if (Inst->isSafeToRemove())
        Inst->eraseFromParent();
    }
  }
  void runOnBasicBlock(BasicBlock &B) {
    bool AlgebraicFlag;
    std::vector<Instruction *> DeleteInst;
    for (auto &Inst : B) {
      if (Inst.isBinaryOp()) {
        AlgebraicFlag = true;
        Value *Operand0 = Inst.getOperand(0);
        Value *Operand1 = Inst.getOperand(1);
        ConstantInt *ConstValue0, *ConstValue1;
        if (isa<ConstantInt>(Operand0)) {
          ConstValue0 = dyn_cast<ConstantInt>(Operand0);
        }
        if (isa<ConstantInt>(Operand1)) {
          ConstValue1 = dyn_cast<ConstantInt>(Operand1);
        }

        if (isa<ConstantInt>(Operand0) && isa<ConstantInt>(Operand1)) {
          switch (Inst.getOpcode()) {
          case Instruction::Add: {
            Inst.replaceAllUsesWith(ConstantInt::getSigned(
                Inst.getType(),
                ConstValue0->getSExtValue() + ConstValue1->getSExtValue()));
            break;
          }
          case Instruction::Sub: {
            Inst.replaceAllUsesWith(ConstantInt::getSigned(
                Inst.getType(),
                ConstValue0->getSExtValue() - ConstValue1->getSExtValue()));
            break;
          }
          case Instruction::Mul: {
            Inst.replaceAllUsesWith(ConstantInt::getSigned(
                Inst.getType(),
                ConstValue0->getSExtValue() * ConstValue1->getSExtValue()));
            break;
          }
          case Instruction::SDiv: {
            Inst.replaceAllUsesWith(ConstantInt::getSigned(
                Inst.getType(),
                ConstValue0->getSExtValue() / ConstValue1->getSExtValue()));
            break;
          }
          default: {
            AlgebraicFlag = false;
          }
          }
          if (AlgebraicFlag) {
            DeleteInst.push_back(&Inst);
          }
          continue;
        }

        switch (Inst.getOpcode()) {
        case Instruction::Add: {
          if (isa<ConstantInt>(Operand0) &&
              (ConstValue0->getSExtValue() == 0)) {
            // 0+x = x
            Inst.replaceAllUsesWith(Operand1);
          } else if (isa<ConstantInt>(Operand1) &&
                     (ConstValue1->getSExtValue() == 0)) {
            // x+0 = x
            Inst.replaceAllUsesWith(Operand0);
          } else {
            AlgebraicFlag = false;
          }
          break;
        }
        case Instruction::Sub: {
          if (isa<ConstantInt>(Operand1) &&
              (ConstValue1->getSExtValue() == 0)) {
            // x-0 = x
            Inst.replaceAllUsesWith(Operand0);
          } else if (Operand0 == Operand1) {
            // x-x = 0
            Inst.replaceAllUsesWith(ConstantInt::getSigned(Inst.getType(), 0));
          } else {
            AlgebraicFlag = false;
          }
          break;
        }
        case Instruction::Mul: {
          if (isa<ConstantInt>(Operand0) &&
              (ConstValue0->getSExtValue() == 1)) {
            // 1*x = x
            Inst.replaceAllUsesWith(Operand1);
          } else if (isa<ConstantInt>(Operand1) &&
                     (ConstValue1->getSExtValue() == 1)) {
            // x*1 = x
            Inst.replaceAllUsesWith(Operand0);
          } else {
            AlgebraicFlag = false;
          }
          break;
        }
        case Instruction::SDiv: {
          // x/1 = x
          if (isa<ConstantInt>(Operand1) &&
              (ConstValue1->getSExtValue() == 1)) {
            Inst.replaceAllUsesWith(Operand0);
          } else if (Operand0 == Operand1) {
            // x/x = 1
            Inst.replaceAllUsesWith(ConstantInt::getSigned(Inst.getType(), 1));
          } else {
            AlgebraicFlag = false;
          }
          break;
        }
        default: {
          AlgebraicFlag = false;
          break;
        }
        }

        if (AlgebraicFlag) {
          DeleteInst.push_back(&Inst);
        }
      }
    }
    deleteInstruction(DeleteInst);
  }

public:
  static char ID;

  AlgebraicIdentity() : FunctionPass(ID) {}

  /**
   * @todo(cscd70) Please complete the methods below.
   * x+0 = 0+x = x*1 = 1*x = x = x-0 = x/1
   */
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnFunction(Function &F) override {
    for (auto &Item : F) {
      runOnBasicBlock(Item);
    }
    return false;
  }
}; // class AlgebraicIdentity

char AlgebraicIdentity::ID = 0;
RegisterPass<AlgebraicIdentity> X("algebraic-identity",
                                  "CSCD70: Algebraic Identity");

} // anonymous namespace
