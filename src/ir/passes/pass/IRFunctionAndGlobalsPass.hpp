#pragma once


#include<iostream>
#include "ir/IRModule.h"
#include "ir/IRInstructions.h"
using namespace std;

/*
 * IRFunctionAndGlobalsPass class
 */
template<class RET, class PARAM>
class IRFunctionAndGlobalsPass
{
  public:
    void run(IRModule &module, PARAM param) {
      for (auto &global : module.globalVariables) {
        visitGlobal(&get<IRGlobalVar>(global), param);
      }
      for (auto &func : module.functions) {
        visitFunction(&func, param);
      }
    }


    virtual RET visitGlobal(IRGlobalVar *var, PARAM param)
    {}

    virtual RET visitFunction(IRFunction *func, PARAM param)
    {}
};


