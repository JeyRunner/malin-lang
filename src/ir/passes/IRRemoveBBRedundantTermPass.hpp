#pragma once

#include <iostream>
#include <ranges>
#include "ir/passes/pass/IRBasicBlockPass.hpp"
using namespace std;

/**
 * when a basic block is terminated by a return or jump instruction this pass remove all subsequent instructions from the bb.
 */
class IRRemoveBBRedundantTermPass: public IRBasicBlockPass<void, int>
{
  public:
    void run(IRModule &module) {
      IRBasicBlockPass::run(module, 0);
    }

    void visitBasicBlock(IRBasicBlock *bb, int param) override {
      auto firstBBTerminationInstruction = ranges::find_if(bb->instructions, irValueIsBBTerminationInstruction);
      if (firstBBTerminationInstruction != bb->instructions.end()) {
        // remove all instructions after first termination instruction
        firstBBTerminationInstruction++;
        bb->instructions.erase(firstBBTerminationInstruction, bb->instructions.end());
      }
    }


    static bool irValueIsBBTerminationInstruction(IRValueVar &instruction) {
      return holds_alternative<IRReturn>(instruction)
          || holds_alternative<IRJump>(instruction)
          || holds_alternative<IRConditionalJump>(instruction);
    }
};


