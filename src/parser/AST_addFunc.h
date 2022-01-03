
#include "AST.h"

// LangTypes
string ClassType::toString() {
  return "class_" + classDeclaration->name;
}
