#pragma once
//#include "IRFunction.h"
//#include "IRInstructions.h"

#include <utility>
#include <variant>
#include "IRElement.h"
#include "IRTypes.h"


class IRValueInvalid;
class IRConstNumberI32;
class IRConstNumberF32;
class IRConstBoolean;
class IRBuildInTypeAllocation;
class IRNumberCalculationBinary;
class IRNumberCompareBinary;
class IRGlobalVar;
class IRLoad;
class IRStore;
class IRReturn;
class IRJump;
class IRConditionalJump;
class IRCall;
class IRFunctionArgument;
class IRLogicalNot;


/**
 * IR Value base class.
 */
class IRValue: public IRElement {
  public:
    IRType type = IRTypeInvalid();

    IRValue()
    {}

    explicit IRValue(const string &name) : IRElement(name)
    {}
};


/**
 * Non valid Ir value.
 */
class IRValueInvalid: public IRValue {
};

/**
 * Comment Ir value.
 * Used just for debugging, has no other functionality.
 */
class IRValueComment: public IRValue {
  public:
    string comment;

    explicit IRValueComment(string comment) : comment(std::move(comment)) {
      type = IRTypeVoid();
    }
};


/**
 * Represents any IRValue.
 */
using IRValueVar = variant<
    IRValueInvalid,
    IRValueComment,
    IRConstNumberI32,
    IRConstBoolean,
    IRLogicalNot,
    IRBuildInTypeAllocation,
    IRLoad,
    IRStore,
    IRNumberCalculationBinary,
    IRNumberCompareBinary,
    IRGlobalVar,
    IRReturn,
    IRJump,
    IRConditionalJump,
    IRCall,
    IRFunctionArgument,
    IRConstNumberF32
>;