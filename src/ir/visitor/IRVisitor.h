#pragma once
#include "ir/IRModule.h"


/**
 * To create a visitor for ir values extend the IRValueVisitor class.
 */
class IRVisitor {
  public:

    /**
     * Visitor for ir values
     * @tparam RET return type of each visit method
     * @tparam PARAM additional parameter type for each visit method
     */
    template<class RET, class PARAM>
    class IRValueVisitor {
      public:
        /**
         * Generic visit will cause a call to the corresponding visit method based on the ir value type.
         */
        RET visitIRValue(IRValueVar &value, PARAM param) {
          auto v = IRValueVisitorIntern<RET, PARAM>(this, param);
          return std::visit(v, value);
        }

        virtual RET visit(IRValueInvalid &val, PARAM param) = 0;
        virtual RET visit(IRGlobalVar &val, PARAM param) = 0;
        virtual RET visit(IRBuildInTypeAllocation &val, PARAM param) = 0;
        virtual RET visit(IRConstNumberI32 &val, PARAM param) = 0;
        virtual RET visit(IRConstBoolean &val, PARAM param) = 0;
        virtual RET visit(IRNumberCalculationBinary &val, PARAM param) = 0;
        virtual RET visit(IRNumberCompareBinary &val, PARAM param) = 0;
        virtual RET visit(IRLoad &val, PARAM param) = 0;
        virtual RET visit(IRStore &val, PARAM param) = 0;
        virtual RET visit(IRReturn &val, PARAM param) = 0;
    };


    /**
     * Internal visitor class, used for std::visit(...).
     * Contains one operator() for each IRValueVar type
     */
    template<class RET, class PARAM>
    class IRValueVisitorIntern {
      public:
        IRValueVisitor<RET, PARAM> &visitor;
        PARAM param;

        IRValueVisitorIntern(IRValueVisitor<RET, PARAM> *visitor, PARAM param):
          visitor(*visitor), param(param) {
        }

        RET operator() (IRValueInvalid &val) { return visitor.visit(val, param); }
        RET operator() (IRGlobalVar &val) { return visitor.visit(val, param); }

        RET operator() (IRBuildInTypeAllocation &val) { return visitor.visit(val, param); }
        RET operator() (IRConstNumberI32 &val) { return visitor.visit(val, param); }
        RET operator() (IRConstBoolean &val) { return visitor.visit(val, param); }
        RET operator() (IRNumberCalculationBinary &val) { return visitor.visit(val, param); }
        RET operator() (IRNumberCompareBinary &val) { return visitor.visit(val, param); }
        RET operator() (IRLoad &val) { return visitor.visit(val, param); }
        RET operator() (IRStore &val) { return visitor.visit(val, param); }
        RET operator() (IRReturn &val) { return visitor.visit(val, param); }
    };
};



