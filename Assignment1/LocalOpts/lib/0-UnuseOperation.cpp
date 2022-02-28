#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llvm;

namespace {

class UnuseOpt final : public FunctionPass {
private:
  void deleteInstruction(std::vector<Instruction *> Insts) {
    for (auto &Inst : Insts) {
      if (Inst->isSafeToRemove())
        Inst->eraseFromParent();
    }
  }
  void runOnBasicBlock(BasicBlock &B) {
    std::vector<Instruction *> DeleteInst;
    for (auto &Inst : B) {
      if (Inst.use_empty()) {
        DeleteInst.push_back(&Inst);
      }
    }
    deleteInstruction(DeleteInst);
  }

public:
  static char ID;

  UnuseOpt() : FunctionPass(ID) {}

  /**
   * @todo(cscd70) Please complete the methods below.
   */
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnFunction(Function &F) override {
    for (auto &Item : F) {
      runOnBasicBlock(Item);
      runOnBasicBlock(Item);
    }
    return false;
  }
}; // class MultiInstOpt

char UnuseOpt::ID = 0;
RegisterPass<UnuseOpt> X("unuse-operation",
                         "CSCD70: UnuseOperation Optimization");

} // anonymous namespace
