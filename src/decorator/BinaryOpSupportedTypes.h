#pragma once

/**
 * Computes the result type of a binary operation
 * @param operandsType the type of both operands
 * @param operation
 * @return InvalidType if operandsType does not support binary op,
 *          otherwise type of binary operation result
 */
unique_ptr<LangType> binaryOperationResultType(LangType *operandsType, BinaryExpressionOp operation) {
  if (operandsType->isNumericalType()) {
    // number
    switch (operation) {
      case EXPR_OP_GREATER_THEN:
      case EXPR_OP_GREATER_EQUALS_THEN:
      case EXPR_OP_LESS_THEN:
      case EXPR_OP_LESS_EQUALS_THEN:
      case EXPR_OP_EQUALS:
      case EXPR_OP_NOT_EQUALS:
        return make_unique<BuildInType>(BuildIn_bool);
      case Expr_Op_Plus:
      case Expr_Op_Minus:
      case Expr_Op_Divide:
      case Expr_Op_Multiply:
        return operandsType->clone();
      default:
        return make_unique<InvalidType>();
    }
  }
  else if (auto ty = dynamic_cast<BuildInType*>(operandsType)) {
    // bool
    if (ty->type == BuildIn_bool) {
      switch (operation) {
        case EXPR_OP_LOGIC_OR:
        case EXPR_OP_LOGIC_AND:
          return make_unique<BuildInType>(BuildIn_bool);
        case EXPR_OP_EQUALS:
        case EXPR_OP_NOT_EQUALS:
        default:
          return make_unique<InvalidType>();
      }
    }
  }

  return make_unique<InvalidType>();
}