#pragma once
#include <utility>

#include "IRElement.h"
#include "IRValueVar.h"
#include "IRTypes.h"


class IRBasicBlock;


/**
 * Intermediate representation base class.
 * Contains basic blocks.
 */
class IRFunction: public IRElement {
  public:
    string name;
    IRType returnType = IRTypeInvalid();

    bool isExtern = false;
    list<IRBasicBlock> basicBlocks;


    explicit IRFunction(string name):
      name(std::move(name))
      {}

};


/**
 * Basic blocks contain multiple instructions.
 * The last instruction is either a return or a jump instruction.
 */
class IRBasicBlock: public IRElement  {
  public:
    string name;
    list<IRValueVar> instructions;

    /// The function the BasicBlock belongs to
    IRFunction *function= nullptr;

    explicit IRBasicBlock(string name) : name(std::move(name)) {
    }
};