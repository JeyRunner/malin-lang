#pragma once
#include <utility>

#include "IRElement.h"
#include "IRValueVar.h"
#include "IRTypes.h"


class IRBasicBlock;
class IRFunctionArgument;
class FunctionParamDeclaration;


/**
 * Intermediate representation base class.
 * Contains basic blocks.
 */
class IRFunction: public IRElement {
  public:
    string name;
    IRType returnType = IRTypeInvalid();

    bool isExtern = false;
    /**
     * all argument indicies are fixed, when calling use the index of an argument to set its value. @see IRCall
     * contains IRFunctionArgument.
     * @note: do  not change the length, this will copy the arguments and will invalidate other pointers to the arguments
     */
    vector<IRValueVar> arguments;
    list<IRBasicBlock> basicBlocks;

    IRFunctionArgument& getArgument(int index);


    explicit IRFunction(string name):
      name(std::move(name))
      {}

};


/**
 * Basic blocks contain multiple instructions.
 * The last instruction is either a return or a sume jump instruction.
 */
class IRBasicBlock: public IRElement  {
  public:
    list<IRValueVar> instructions;

    /// The function the BasicBlock belongs to
    IRFunction *function= nullptr;

    explicit IRBasicBlock(string name) : IRElement(std::move(name)) {
    }
};


/**
 * Function argument.
 * Contains the arg index, arg name and arg type.
 */
class IRFunctionArgument: public IRValue  {
  public:
    /// is a Constant Value, this is optional, if arg has no default value this is null
    IRValueVar *initValue = nullptr;

    /// The function the IRFunctionArgument belongs to
    IRFunction *function= nullptr;

    FunctionParamDeclaration *astFunctionParamDeclaration = nullptr;

    explicit IRFunctionArgument(const string& name) : IRValue(name) {
    }
};