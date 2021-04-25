#pragma once
#include "IRFunction.h"
#include "IRGlobal.h"

/**
 * Intermediate representation Module.
 * Contains global ir definitions: global variables, functions and classes.
 */
class IRModule {
  public:
    string sourceFileName;
    bool isValid = false;


    /**
     * Function of the module.
     * NOTE: needs to stay a list to ensure pointers to elements stay valid after changing the list
     *       this does not hold for vectors
     */
    list<IRFunction> functions;

    /**
     * Global variables.
     * All elements are objects of IRGlobalVar
     */
    list<IRValueVar> globalVariables;
    /// contains init values for the global vars, all are constant values
    list<IRValueVar> globalVariablesInitValues;


    IRModule() {}

    /// TODO: for implementing the copy of a module all value references have to be refreshed
    IRModule(IRModule const &m) = delete;
};



