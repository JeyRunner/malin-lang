#pragma once
//#include "IRFunction.h"
//#include "IRInstructions.h"

#include <variant>
#include "IRElement.h"
#include "IRTypes.h"


class IRValueInvalid;
class IRConstNumberI32;
class IRConstBoolean;
class IRBuildInTypeAllocation;
class IRNumberCalculationBinary;
class IRNumberCompareBinary;
class IRGlobalVar;
class IRLoad;
class IRStore;
class IRReturn;


/**
 * IR Value base class.
 */
class IRValue: public IRElement {
  public:
    IRType type = IRTypeInvalid();
};


/**
 * Non valid Ir value.
 */
class IRValueInvalid {
};


/**
 * Represents any IRValue.
 */
using IRValueVar = variant<
    IRValueInvalid,
    IRConstNumberI32,
    IRConstBoolean,
    IRBuildInTypeAllocation,
    IRLoad,
    IRStore,
    IRNumberCalculationBinary,
    IRNumberCompareBinary,
    IRGlobalVar,
    IRReturn
>;