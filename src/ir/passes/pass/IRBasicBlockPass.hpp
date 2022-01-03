#pragma once

#include "IRFunctionAndGlobalsPass.hpp"
using namespace std;

/*
 * IRFunctionAndGlobalsPass class
 */
template<class RET, class PARAM>
class IRBasicBlockPass: public IRFunctionAndGlobalsPass<RET, PARAM>
{
  public:
    virtual RET visitFunction(IRFunction *func, PARAM param) {
      for (auto &bb : func->basicBlocks) {
        visitBasicBlock(&bb, param);
      }
    }

    virtual RET visitBasicBlock(IRBasicBlock *bb, PARAM param)
    {}
};


