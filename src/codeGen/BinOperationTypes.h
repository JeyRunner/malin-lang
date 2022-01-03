#pragma once
using namespace llvm;


struct CompareInfo {
    CmpInst::Predicate pred;
    bool isCompare;
    CompareInfo(): isCompare(false) {}
    CompareInfo(CmpInst::Predicate predi): pred(predi), isCompare(true) {}
};

/**
 * Get the llvm compare operation from BinaryExpressionOp
 * @param operation op type of the BinaryExpression
 * @param operandType type of the operands
 */
CompareInfo getBinaryOperationCompare(BinaryExpressionOp operation, BUILD_IN_TYPE operandType) {
  switch (operation) {
    case EXPR_OP_EQUALS:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_EQ);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_OEQ);
      } break;
    case EXPR_OP_NOT_EQUALS:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_NE);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_ONE);
      } break;
    case EXPR_OP_GREATER_THEN:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_SGT);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_OGT);
      } break;
    case EXPR_OP_GREATER_EQUALS_THEN:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_SGE);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_OGE);
      } break;
    case EXPR_OP_LESS_THEN:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_SLT);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_OLT);
      } break;
    case EXPR_OP_LESS_EQUALS_THEN:
      switch (operandType) {
        case BuildIn_i32:   return CompareInfo(CmpInst::Predicate::ICMP_SLE);
        case BuildIn_f32:   return CompareInfo(CmpInst::Predicate::FCMP_OLE);
      } break;
    default:
      return CompareInfo();
  }
  return CompareInfo();
}