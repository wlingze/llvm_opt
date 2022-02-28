#include <cstddef>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>
#include <vector>

using namespace llvm;

namespace {

class StrengthReduction final : public FunctionPass {
private:
  void deleteInst(std::vector<Instruction *> Insts) {
    for (auto &Inst : Insts) {
      if (Inst->isSafeToRemove())
        Inst->eraseFromParent();
    }
  }
  size_t getShift(unsigned Num) {
    if ((Num & (Num - 1)))
      return 0;
    size_t Shift = 0;
    while (Num != 1) {
      Num >>= 1;
      Shift++;
    }
    return Shift;
  }

  void runOnBasicBlock(BasicBlock &B) {
    std::vector<Instruction *> DeleteInstruction;
    bool StregthFlag;
    for (auto &Inst : B) {

      if (Inst.isBinaryOp()) {
        StregthFlag = true;
        Value *Operand0 = Inst.getOperand(0);
        Value *Operand1 = Inst.getOperand(1);
        ConstantInt *ConsVal0, *ConsVal1;
        size_t Shift;
        if (isa<ConstantInt>(Operand0))
          ConsVal0 = dyn_cast<ConstantInt>(Operand0);
        if (isa<ConstantInt>(Operand1))
          ConsVal1 = dyn_cast<ConstantInt>(Operand1);

        switch (Inst.getOpcode()) {
        case Instruction::Mul: {
          if (isa<ConstantInt>(Operand0) &&
              (Shift = getShift(ConsVal0->getZExtValue()))) {
            BinaryOperator *NewInst = BinaryOperator::Create(
                Instruction::Shl, Operand1,
                ConstantInt::getSigned(Inst.getType(), Shift), "shl", &Inst);
            Inst.replaceAllUsesWith(NewInst);
          } else if (isa<ConstantInt>(Operand1) &&
                     (Shift = getShift(ConsVal1->getZExtValue()))) {
            BinaryOperator *NewInst = BinaryOperator::Create(
                Instruction::Shl, Operand0,
                ConstantInt::getSigned(Inst.getType(), Shift), "shl", &Inst);
            Inst.replaceAllUsesWith(NewInst);
          } else {
            StregthFlag = false;
          }
          break;
        }
        case Instruction::SDiv: {
          if (isa<ConstantInt>(Operand1) &&
              (Shift = getShift(ConsVal1->getZExtValue()))) {
            BinaryOperator *NewInst = BinaryOperator::Create(
                Instruction::LShr, Operand0,
                ConstantInt::getSigned(Inst.getType(), Shift), "shr", &Inst);
            Inst.replaceAllUsesWith(NewInst);
          } else {
            StregthFlag = false;
          }
          break;
        }
        default: {
          StregthFlag = false;
        }
        }

        if (StregthFlag) {
          DeleteInstruction.push_back(&Inst);
        }
      }
    }
    deleteInst(DeleteInstruction);
  }

public:
  static char ID;

  StrengthReduction() : FunctionPass(ID) {}

  /**
   * @todo(cscd70) Please complete the methods below.
   * x*4 = x<<2;
   */
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnFunction(Function &F) override {
    for (auto &Item : F) {
      runOnBasicBlock(Item);
    }
    return false;
  }
}; // class StrengthReduction

char StrengthReduction::ID = 0;
RegisterPass<StrengthReduction> X("strength-reduction",
                                  "CSCD70: Strength Reduction");

} // anonymous namespace
