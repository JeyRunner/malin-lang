#pragma once
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include <utility>

#include "ir/IRModule.h"

/**
 * Create functions basic blocks and instructions for a IR module.
 */
class IRBuilder {
  private:
    /// current function to append basic blocks to
    IRFunction *currentFunction = nullptr;

    /// current BasicBlock to append instructions to
    IRBasicBlock *currentBasicBlock = nullptr;


  public:
    /// this needs to be set before calling any builder function
    IRModule *module = nullptr;


    /**
     * Create an IRBuilder.
     */
    IRBuilder(IRModule &module) : module(&module) {
    }
    IRBuilder() {
    }

    /**
     * Set the BasicBlock subsequent instructions are added to.
     * This also changes the current function to the one the BasicBlock belongs to.
     * @param basicBlock
     */
    void setInsertionBasicBlock(IRBasicBlock &basicBlock) {
      this->currentFunction = basicBlock.function;
      this->currentBasicBlock = &basicBlock;
    }

    /**
    * Get the BasicBlock subsequent instructions are added to.
    */
    IRBasicBlock &getInsertionBasicBlock() {
      return *this->currentBasicBlock;
    }


    /**
     * Add a function to the module.
     * This changes the current function to the new created one,
     * BasicBlocks will be appended to this new crated function.
     * Also a entry BasicBlock is created for this new function and
     * set as insertion point for instructions
     * (subsequent add instruction calls will add instructions to this block, until another BasicBlock ist selected)
     *
     * @param name name of the added function
     * @return the new added function
     */
    IRFunction &Function(string name) {
      module->functions.push_back(IRFunction(std::move(name)));
      IRFunction &f = module->functions.back();
      this->currentFunction = &f;
      this->BasicBlock("entry");
      return f;
    }


    /**
     * Add a function argument to the current function.
     *
     * @param name name of the added function argument
     * @return the new added function argument
     */
    IRFunctionArgument &FunctionArgument(string name) {
      this->currentFunction->arguments.emplace_back(IRFunctionArgument(name));
      IRFunctionArgument &arg = get<IRFunctionArgument>(this->currentFunction->arguments.back());
      arg.function = currentFunction;
      return arg;
    }


    /**
     * Add a global variable to the module.
     *
     * @param name name of the added global var
     * @return the new added global var
     */
    IRGlobalVar &GlobalVar(string name) {
      module->globalVariables.push_back(IRGlobalVar(std::move(name)));
      return get<IRGlobalVar>(module->globalVariables.back());
    }


    /**
     * Add a new BasicBlock to the current function.
     * This sets the current BasicBlock to the new created.
     * Subsequent add instruction calls will add instructions to this block, until another BasicBlock ist selected.
     * @param name the new BasicBlocks name
     * @return the new BasicBlock
     */
    IRBasicBlock &BasicBlock(string name) {
      currentFunction->basicBlocks.emplace_back(IRBasicBlock(std::move(name)));
      IRBasicBlock &block = currentFunction->basicBlocks.back();
      block.function = currentFunction;
      currentBasicBlock = &block;
      return block;
    }


    /**
     * Add a new instruction to the current BasicBlock.
     * @note the module will take ownership of the instruction object,
     *       for further changes to the returned pointer to the instruction has to be used.
     * @param instruction the instruction to add
     * @return the new pointer to th added instruction.
     */
    template<class T>
    T &Instruction(T instruction) {
      currentBasicBlock->instructions.emplace_back(move(instruction));
      return get<T>(currentBasicBlock->instructions.back());
    }
};



