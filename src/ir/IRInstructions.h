#pragma once
#include <utility>

#include "IRElement.h"
#include "IRValueVar.h"
#include "IRGlobal.h"
#include "IRFunction.h"
#include "builder/exceptions.h"
#include "../parser/Types.h"
#include "../parser/AST.h"


/**
 * Const number.
 */
class IRConstNumberI32: public IRValue {
  public:
    int32_t value = 0;

    IRConstNumberI32() {
      type = IRTypeBuildIn(BuildIn_i32);
    }
};

/**
 * Const number.
 */
class IRConstNumberF32: public IRValue {
  public:
    float_t value = 0;

    IRConstNumberF32() {
      type = IRTypeBuildIn(BuildIn_f32);
    }
};


/**
 * Const bool.
 */
class IRConstBoolean: public IRValue {
  public:
    bool value = false;

    IRConstBoolean() {
      type = IRTypeBuildIn(BuildIn_bool);
    }
};


/**
 * Allocate buildIn type.
 */
class IRBuildInTypeAllocation: public IRValue {
  public:
    IRBuildInTypeAllocation(BUILD_IN_TYPE buildInType) {
      type = IRTypePointer(IRTypeBuildIn(buildInType));
    }
};


/**
 * Return a value from a function.
 * the returnValue is optional.
 */
class IRReturn: public IRValue {
  public:
    // optional
    IRValueVar* returnValue = nullptr;

    explicit IRReturn(IRValueVar *returnValue) : returnValue(returnValue) {
      type = IRTypeVoid();
    }
};





/**
 * Number unary operation.
 */
class IRNumberCalculationUnary: public IRValue {
  public:
};



enum class IR_NUMBER_CALCULATION_BINARY_OP {
    INVALID,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
};

/**
 * Number binary operation.
 */
class IRNumberCalculationBinary: public IRValue {
  public:
    IR_NUMBER_CALCULATION_BINARY_OP op = IR_NUMBER_CALCULATION_BINARY_OP::INVALID;

    IRValueVar* lhs = nullptr;
    IRValueVar* rhs = nullptr;
};

static IR_NUMBER_CALCULATION_BINARY_OP IR_NUMBER_CALCULATION_BINARY_OP_fromBinaryExpressionOp(BinaryExpressionOp op, ASTNode *astNode) {
  switch (op) {
    case Expr_Op_Plus:
      return IR_NUMBER_CALCULATION_BINARY_OP::ADD;
    case Expr_Op_Minus:
      return IR_NUMBER_CALCULATION_BINARY_OP::SUBTRACT;
    case Expr_Op_Divide:
      return IR_NUMBER_CALCULATION_BINARY_OP::DIVIDE;
    case Expr_Op_Multiply:
      return IR_NUMBER_CALCULATION_BINARY_OP::MULTIPLY;

    case Expr_Op_Invalid:
    default:
      throw IRGenException("incompatible binary operation for binary number calculation: " + toString(op), astNode->location);
  }
}

static string toString(IR_NUMBER_CALCULATION_BINARY_OP op) {
  switch (op) {
    case IR_NUMBER_CALCULATION_BINARY_OP::INVALID:
      return "INVALID";
    case IR_NUMBER_CALCULATION_BINARY_OP::ADD:
      return "add+";
    case IR_NUMBER_CALCULATION_BINARY_OP::SUBTRACT:
      return "subtract-";
    case IR_NUMBER_CALCULATION_BINARY_OP::MULTIPLY:
      return "multiply*";
    case IR_NUMBER_CALCULATION_BINARY_OP::DIVIDE:
      return "divide/";
  }
}







enum class IR_NUMBER_COMPARE_BINARY_OP {
    INVALID,
    EQUALS,
    NOT_EQUALS,
    GREATER,
    GREATER_EQUALS,
    LESS,
    LESS_EQUALS
};

/**
 * Number binary operation.
 */
class IRNumberCompareBinary: public IRValue {
  public:
    IR_NUMBER_COMPARE_BINARY_OP op = IR_NUMBER_COMPARE_BINARY_OP::INVALID;

    IRValueVar* lhs = nullptr;
    IRValueVar* rhs = nullptr;
};

static IR_NUMBER_COMPARE_BINARY_OP IR_NUMBER_COMPARE_BINARY_OP_fromBinaryExpressionOp(BinaryExpressionOp op, ASTNode *astNode) {
  switch (op) {
    case EXPR_OP_EQUALS:
      return IR_NUMBER_COMPARE_BINARY_OP::EQUALS;
    case EXPR_OP_NOT_EQUALS:
      return IR_NUMBER_COMPARE_BINARY_OP::NOT_EQUALS;

    case EXPR_OP_GREATER_THEN:
      return IR_NUMBER_COMPARE_BINARY_OP::GREATER;
    case EXPR_OP_GREATER_EQUALS_THEN:
      return IR_NUMBER_COMPARE_BINARY_OP::GREATER_EQUALS;

    case EXPR_OP_LESS_THEN:
      return IR_NUMBER_COMPARE_BINARY_OP::LESS;
    case EXPR_OP_LESS_EQUALS_THEN:
      return IR_NUMBER_COMPARE_BINARY_OP::LESS_EQUALS;

    case Expr_Op_Invalid:
    default:
      throw IRGenException("incompatible binary operation for binary number comparison: " + toString(op), astNode->location);
  }
}

static string toString(IR_NUMBER_COMPARE_BINARY_OP op) {
  switch (op) {
    case IR_NUMBER_COMPARE_BINARY_OP::INVALID:
      return "INVALID";
    case IR_NUMBER_COMPARE_BINARY_OP::EQUALS:
      return "==";
    case IR_NUMBER_COMPARE_BINARY_OP::NOT_EQUALS:
      return "!=";
    case IR_NUMBER_COMPARE_BINARY_OP::GREATER:
      return ">";
    case IR_NUMBER_COMPARE_BINARY_OP::GREATER_EQUALS:
      return ">=";
    case IR_NUMBER_COMPARE_BINARY_OP::LESS:
      return "<";
    case IR_NUMBER_COMPARE_BINARY_OP::LESS_EQUALS:
      return "<=";
  }
}



enum class IR_BOOLEAN_BINARY_OP {
    INVALID,
    AND,
    OR
};

/**
 * Boolean binary operation.
 */
class IRBooleanOperationBinary: public IRValue {
  public:
    IR_BOOLEAN_BINARY_OP op = IR_BOOLEAN_BINARY_OP::INVALID;

    IRValueVar* lhs = nullptr;
    IRValueVar* rhs = nullptr;
};

static IR_BOOLEAN_BINARY_OP IR_BOOLEAN_BINARY_OP_fromBinaryExpressionOp(BinaryExpressionOp op, ASTNode *astNode) {
  switch (op) {
    case EXPR_OP_LOGIC_AND:
      return IR_BOOLEAN_BINARY_OP::AND;
    case EXPR_OP_LOGIC_OR:
      return IR_BOOLEAN_BINARY_OP::OR;
    case Expr_Op_Invalid:
    default:
      throw IRGenException("incompatible binary boolean operation: " + toString(op), astNode->location);
  }
}

static string toString(IR_BOOLEAN_BINARY_OP op) {
  return string(magic_enum::enum_name(op));
}




/**
 * Logical not instruction: negates boolean value
 */
class IRLogicalNot: public IRValue {
  public:
    IRValueVar *negateValue;

    explicit IRLogicalNot(IRValueVar *negateValue): negateValue(negateValue)
    {}
};






/**
 * Load instruction: dereferences a pointer.
 */
class IRStore: public IRValue {
  public:
    IRValueVar *destinationPointer;
    IRValueVar *valueToStore;

    explicit IRStore(IRValueVar *destinationPointerValue, IRValueVar *valueToStore)
        : destinationPointer(destinationPointerValue), valueToStore(valueToStore) {
      type = IRTypeVoid();
    }
};


/**
 * Load instruction: dereferences a pointer.
 */
class IRLoad: public IRValue {
  public:
    IRValueVar *valueToLoad;

    explicit IRLoad(IRValueVar *valueToLoad);
};



/**
 * Jump instruction: jump to another basic block.
 * This can only be the last instruction of a basic block.
 * The last instruction of a Basic block either needs to be some jump instruction or return.
 */
class IRJump: public IRValue {
  public:
    IRBasicBlock *jumpToBB;

    /**
     * @param jumpTo jump to this basic block.
     */
    explicit IRJump(IRBasicBlock *jumpTo): jumpToBB(jumpTo) {
      type = IRTypeVoid();
    }
};


/**
 * Conditional Jump instruction: based on a condition value either jump to one basic block or another.
 * This can only be the last instruction of a basic block.
 * The last instruction of a Basic block either needs to be some jump instruction or return.
 */
class IRConditionalJump: public IRValue {
  public:
    IRValueVar *conditionValue = nullptr;
    IRBasicBlock *jumpToWhenFalseBB = nullptr;
    IRBasicBlock *jumpToWhenTrueBB = nullptr;

    /**
     * @param condition perform jump based on this condition value.
     * @param jumpToWhenTrue when condition is true, jump to this basic block.
     * @param jumpToWhenFalse when condition is false, jump to this basic block.
     */
    explicit IRConditionalJump(IRValueVar *condition, IRBasicBlock *jumpToWhenTrue, IRBasicBlock *jumpToWhenFalse)
    : conditionValue(condition), jumpToWhenFalseBB(jumpToWhenFalse), jumpToWhenTrueBB(jumpToWhenTrue)  {
      type = IRTypeVoid();
    }

    /**
     * @param condition perform jump based on this condition value.
     */
    explicit IRConditionalJump(IRValueVar *condition)
        : conditionValue(condition) {
      type = IRTypeVoid();
    }
};


/**
 * Call instruction: call a function.
 */
class IRCall: public IRValue {
  public:
    IRFunction *function;

    /**
     * arguments have to be in the right order defined in function.arguments
     * default arguments are filled-in in a separate ir pass, therefore when on value is provided for an argument set it index to nullptr
     */
    vector<IRValueVar*> arguments;

    bool addedDefaultValues = false;

    explicit IRCall(IRFunction *functionToCall, vector<IRValueVar *> arguments)
        : function(functionToCall), arguments(std::move(arguments)) {
      type = functionToCall->returnType;
    }
};