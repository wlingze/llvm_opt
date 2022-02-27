#include <cstddef>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {

class FunctionInfo final : public ModulePass {
public:
  static char ID;

  FunctionInfo() : ModulePass(ID) {}

  // We don't modify the program, so we preserve all analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  virtual bool runOnModule(Module &M) override {
    outs() << "CSCD70 Function Information Pass"
           << "\n";

    /**
     * @todo(cscd70) Please complete this method.
     */
    outs() << "Name\t"
           << "#Args\t"
           << "#Calls\t"
           << "#Blocks\t"
           << "#Insts\t"
           << "\n";
    for (auto Item = M.begin(); Item != M.end(); Item++) {
      outs() << Item->getName() << "\t" << Item->arg_size() << "\t"
             << Item->getNumUses() << "\t" << Item->size() << "\t"
             << Item->getInstructionCount() << "\n";
    }

    return false;
  }
}; // class FunctionInfo

char FunctionInfo::ID = 0;
RegisterPass<FunctionInfo> X("function-info", "CSCD70: Function Information");

} // anonymous namespace
