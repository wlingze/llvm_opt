#include <cstdint>
#include <cstring>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

class MultiInstOpt final : public FunctionPass {
private:
  std::map<std::string, Instruction *> InstMap;

  void deleteInst(std::vector<Instruction *> Insts) {
    for (auto &Inst : Insts) {
      if (Inst->isSafeToRemove())
        Inst->eraseFromParent();
    }
  }

  std::string getValue(Value *V) {
    if (isa<ConstantInt>(V)) {
      return std::to_string(dyn_cast<ConstantInt>(V)->getSExtValue());
    }
    return V->getName().str();
  }

  Instruction *findInst(std::string op) {
    auto search = InstMap.find(op);
    if (search == InstMap.end()) {
      return nullptr;
    }
    return search->second;
  }

  bool canOpt(Instruction &Inst) {
    if (!Inst.isBinaryOp()) {
      return false;
    }
    Value *Operand0 = Inst.getOperand(0);
    Value *Operand1 = Inst.getOperand(1);

    return ((isa<ConstantInt>(Operand0) && isa<Instruction>(Operand1)) ||
            (isa<Instruction>(Operand0) && isa<ConstantInt>(Operand1)));
  }

  void runOnBasicBlock(BasicBlock &B) {
    std::vector<Instruction *> DeleteInst;
    bool MultiInstFlag;
    for (auto &Inst : B) {
      if (Inst.isBinaryOp()) {

        Value *Operand0 = Inst.getOperand(0);
        Value *Operand1 = Inst.getOperand(1);

        std::string str = getValue(Operand0) + " " + Inst.getOpcodeName() +
                          " " + getValue(Operand1);

        Instruction *search;
        if ((search = findInst(str)) != nullptr) {
          Inst.replaceAllUsesWith(search);
          DeleteInst.push_back(&Inst);
          continue;
        } else {
          InstMap[str] = &Inst;
        }

        if (!canOpt(Inst))
          continue;

        Instruction *InstValue0, *InstValue1, *Inst2;
        ConstantInt *ConstValue0, *ConstValue1, *ConstOperand1;
        if (isa<Instruction>(Operand0)) {
          InstValue0 = dyn_cast<Instruction>(Operand0);
          Inst2 = InstValue0;
        }
        if (isa<Instruction>(Operand1)) {
          InstValue1 = dyn_cast<Instruction>(Operand1);
          Inst2 = InstValue1;
        }
        if (isa<ConstantInt>(Operand0)) {
          ConstValue0 = dyn_cast<ConstantInt>(Operand0);
          ConstOperand1 = ConstValue0;
        }
        if (isa<ConstantInt>(Operand1)) {
          ConstValue1 = dyn_cast<ConstantInt>(Operand1);
          ConstOperand1 = ConstValue1;
        }

        if (!canOpt(*Inst2)) {
          continue;
        }
        MultiInstFlag = true;

        Value *Operand2 = Inst2->getOperand(0);
        Value *Operand3 = Inst2->getOperand(1);
        Instruction *InstValue2, *InstValue3, *InstOperand2;
        ConstantInt *ConstValue2, *ConstValue3, *ConstOperand2;
        if (isa<Instruction>(Operand2)) {
          InstValue2 = dyn_cast<Instruction>(Operand2);
          InstOperand2 = InstValue2;
        }
        if (isa<Instruction>(Operand3)) {
          InstValue3 = dyn_cast<Instruction>(Operand3);
          InstOperand2 = InstValue3;
        }
        if (isa<ConstantInt>(Operand2)) {
          ConstValue2 = dyn_cast<ConstantInt>(Operand2);
          ConstOperand2 = ConstValue2;
        }
        if (isa<ConstantInt>(Operand3)) {
          ConstValue3 = dyn_cast<ConstantInt>(Operand3);
          ConstOperand2 = ConstValue3;
        }

        int64_t ConstValueInt1 = ConstOperand1->getSExtValue();
        int64_t ConstValueInt2 = ConstOperand2->getSExtValue();

        Instruction *NewInst;
        switch (Inst.getOpcode()) {
        case Instruction::Add: {

          switch (Inst2->getOpcode()) {
          case Instruction::Add: {
            NewInst = BinaryOperator::Create(
                Instruction::Add, InstOperand2,
                ConstantInt::getSigned(ConstOperand1->getType(),
                                       ConstValueInt1 + ConstValueInt2),
                "new_add", &Inst);
            break;
          }
          case Instruction::Sub: {
            if (ConstOperand2 == ConstValue2) {
              // left
              // inst = con1 + con2 - inst2
              NewInst = BinaryOperator::Create(
                  Instruction::Sub,
                  ConstantInt::getSigned(ConstOperand1->getType(),
                                         ConstValueInt1 + ConstValueInt2),
                  InstOperand2, "new_sub", &Inst);
            } else {
              // right
              // inst = con1 + inst2 - con2
              if (ConstValueInt1 > ConstValueInt2) {
                NewInst = BinaryOperator::Create(
                    Instruction::Add,
                    ConstantInt::getSigned(ConstOperand1->getType(),
                                           ConstValueInt1 - ConstValueInt2),
                    InstOperand2, "new_sub", &Inst);
              } else if (ConstValueInt1 < ConstValueInt2) {
                NewInst = BinaryOperator::Create(

                    Instruction::Sub, InstOperand2,
                    ConstantInt::getSigned(ConstOperand1->getType(),
                                           ConstValueInt2 - ConstValueInt1),
                    "new_sub", &Inst);
              } else {
                NewInst = InstOperand2;
              }
            }
            break;
          }
          default: {
            MultiInstFlag = false;
          }
          }

          break;
        }
        default: {
          MultiInstFlag = false;
        }
        }

        if (MultiInstFlag) {
          Inst.replaceAllUsesWith(NewInst);
          DeleteInst.push_back(&Inst);
        }
      }
    }
    deleteInst(DeleteInst);
  }

public:
  static char ID;

  MultiInstOpt() : FunctionPass(ID) {}

  /**
   * @todo(cscd70) Please complete the methods below.
   */
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnFunction(Function &F) override {
    for (auto &Item : F) {
      runOnBasicBlock(Item);
    }
    return false;
  }
}; // class MultiInstOpt

char MultiInstOpt::ID = 0;
RegisterPass<MultiInstOpt> X("multi-inst-opt",
                             "CSCD70: Multi-Instruction Optimization");

} // anonymous namespace
