#pragma once
#include <utility>

#include "IRElement.h"
#include "IRValueVar.h"
#include "IRTypes.h"


class IRBasicBlock;


/**
 * Global variable.
 */
class IRGlobalVar: public IRValue {
  public:
    /// is a Constant Value
    IRValueVar *initValue = nullptr;

    explicit IRGlobalVar(string name) {
      name = std::move(name);
    }

};