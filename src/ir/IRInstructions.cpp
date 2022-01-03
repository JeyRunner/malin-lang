#include "string"
#include "stdio.h"
#include "IRInstructions.h"
#include "IRFunction.h"
using namespace std;

IRLoad::IRLoad(IRValueVar *valueToLoad) : valueToLoad(valueToLoad) {
  type = *get<IRTypePointer>(((IRValue*)valueToLoad)->type).pointTo;
}


IRFunctionArgument &IRFunction::getArgument(int index) {
  return /*(IRFunctionArgument)*/ (IRFunctionArgument &) (arguments.at(index));
}