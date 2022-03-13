#pragma once // NOLINT(llvm-header-guard)

#include <deque>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

/**
 * @brief A wrapper for binary expressions.
 */
struct Expression {

  const unsigned Opcode;
  const Instruction *Inst;
  const Value *const LHS = nullptr, *const RHS = nullptr;
  Expression(const BinaryOperator &BinaryOp)
      : Opcode(BinaryOp.getOpcode()), Inst(&BinaryOp),
        LHS(BinaryOp.getOperand(0)), RHS(BinaryOp.getOperand(1)) {}

  Expression(const Instruction &I) : Expression(*dyn_cast<BinaryOperator>(&I)) {}

  static bool is(const Instruction &I){
    return isa<BinaryOperator>(I);
  }


  /**
   * @todo(cscd70) Please complete the comparator.
   */
  bool operator==(const Expression &Expr) const {
    if (this == &Expr)
      return true;
    return LHS == Expr.LHS && RHS == Expr.RHS && Opcode == Expr.Opcode;
  }
};

inline raw_ostream &operator<<(raw_ostream &Outs, const Expression &Expr) {
  Outs << "[" << Instruction::getOpcodeName(Expr.Opcode) << " ";
  Expr.LHS->printAsOperand(Outs, false);
  Outs << ", ";
  Expr.RHS->printAsOperand(Outs, false);
  Outs << "]";
  return Outs;
}
