#pragma once
//#include "IRFunction.h"
//#include "IRInstructions.h"

#include <variant>
#include "parser/Types.h"

class IRTypePointer;


/**
 * Non valid type.
 */
class IRTypeInvalid {
};


/**
 * Void type.
 */
class IRTypeVoid {
};


class IRTypeBuildIn {
  public:
    BUILD_IN_TYPE buildInType;

    IRTypeBuildIn(BUILD_IN_TYPE buildInType) : buildInType(buildInType) {
    }
};

/**
 * Type of an IR value.
 */
using IRType = variant<
    IRTypeInvalid,
    IRTypeVoid,
    IRTypeBuildIn,
    IRTypePointer
>;


/**
 * Pointer type that points to an internal IRType.
 */
class IRTypePointer {
  public:
    shared_ptr<IRType> pointTo = nullptr;

    IRTypePointer() = default;
    explicit IRTypePointer(const IRType& pointTo) : pointTo(make_shared<IRType>(pointTo)) {
    }
};








static IRType langTypeToIRType(LangType *langType) {
  if (auto buildIn = dynamic_cast<BuildInType*>(langType)) {
    return IRTypeBuildIn(buildIn->type);
  }

  return IRTypeInvalid();
}

static IRType langTypeToIRType(unique_ptr<LangType> &langType) {
  return langTypeToIRType(langType.get());
}


static string irTypeToString(IRType &type) {
  if (std::holds_alternative<IRTypeBuildIn>(type)) {
    return buildInTypeToString(get<IRTypeBuildIn>(type).buildInType);
  }
  else if (std::holds_alternative<IRTypePointer>(type)) {
    auto &t = get<IRTypePointer>(type);
    return "*" + irTypeToString(*t.pointTo);
  }
  else if (std::holds_alternative<IRTypeInvalid>(type)) {
    return "irTypeInvalid";
  }
}