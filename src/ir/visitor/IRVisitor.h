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
        virtual RET visit(IRValueComment &val, PARAM param) = 0;
        virtual RET visit(IRGlobalVar &val, PARAM param) = 0;
        virtual RET visit(IRBuildInTypeAllocation &val, PARAM param) = 0;
        virtual RET visit(IRConstNumberI32 &val, PARAM param) = 0;
        virtual RET visit(IRConstNumberF32 &val, PARAM param) = 0;
        virtual RET visit(IRConstBoolean &val, PARAM param) = 0;
        virtual RET visit(IRLogicalNot &val, PARAM param) = 0;
        virtual RET visit(IRNumberCalculationBinary &val, PARAM param) = 0;
        virtual RET visit(IRNumberCompareBinary &val, PARAM param) = 0;
        virtual RET visit(IRLoad &val, PARAM param) = 0;
        virtual RET visit(IRStore &val, PARAM param) = 0;
        virtual RET visit(IRReturn &val, PARAM param) = 0;

        virtual RET visit(IRJump &jump, PARAM param) = 0;
        virtual RET visit(IRConditionalJump &condJump, PARAM param) = 0;
        virtual RET visit(IRCall &call, PARAM param) = 0;
        virtual RET visit(IRFunctionArgument &arg, PARAM param) = 0;
    };


    /**
     * Visitor for ir values with two parameters for the visit function
     * @tparam RET return type of each visit method
     * @tparam PARAM1 first additional parameter type for each visit method
     * @tparam PARAM2 second additional parameter type for each visit method
     */
     /*
    template<class RET, class PARAM1, class PARAM2>
    class IRValueVisitor2 {
      public:
        /**
         * Generic visit will cause a call to the corresponding visit method based on the ir value type.
         *-/
        RET visitIRValue(IRValueVar &value, PARAM1 param, PARAM1 param2) {
          auto v = IRValueVisitorIntern2<RET, PARAM1, PARAM2>(this, param, param2);
          return std::visit(v, value);
        }

        virtual RET visit(IRValueInvalid &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRValueComment &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRGlobalVar &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRBuildInTypeAllocation &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRConstNumberI32 &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRConstBoolean &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRNumberCalculationBinary &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRNumberCompareBinary &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRLoad &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRStore &val, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRReturn &val, PARAM1 param, PARAM2 param2) = 0;

        virtual RET visit(IRJump &jump, PARAM1 param, PARAM2 param2) = 0;
        virtual RET visit(IRConditionalJump &condJump, PARAM1 param, PARAM2 param2) = 0;
    };
    */



  private:
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
        RET operator() (IRValueComment &val) { return visitor.visit(val, param); }
        RET operator() (IRGlobalVar &val) { return visitor.visit(val, param); }

        RET operator() (IRBuildInTypeAllocation &val) { return visitor.visit(val, param); }
        RET operator() (IRConstNumberI32 &val) { return visitor.visit(val, param); }
        RET operator() (IRConstNumberF32 &val) { return visitor.visit(val, param); }
        RET operator() (IRConstBoolean &val) { return visitor.visit(val, param); }
        RET operator() (IRLogicalNot &val) { return visitor.visit(val, param); }
        RET operator() (IRNumberCalculationBinary &val) { return visitor.visit(val, param); }
        RET operator() (IRNumberCompareBinary &val) { return visitor.visit(val, param); }
        RET operator() (IRLoad &val) { return visitor.visit(val, param); }
        RET operator() (IRStore &val) { return visitor.visit(val, param); }
        RET operator() (IRReturn &val) { return visitor.visit(val, param); }

        RET operator() (IRJump &val) { return visitor.visit(val, param); }
        RET operator() (IRConditionalJump &val) { return visitor.visit(val, param); }
        RET operator() (IRCall &val) { return visitor.visit(val, param); }
        RET operator() (IRFunctionArgument &val) { return visitor.visit(val, param); }
    };


    /**
     * Internal visitor class, used for std::visit(...).
     * Contains one operator() for each IRValueVar type
     */
     /*
    template<class RET, class PARAM1, class PARAM2>
    class IRValueVisitorIntern2 {
      public:
        IRValueVisitor2<RET, PARAM1, PARAM2> &visitor;
        PARAM1 param1;
        PARAM2 param2;

        IRValueVisitorIntern2(IRValueVisitor2<RET, PARAM1, PARAM2> *visitor, PARAM1 param1, PARAM2 param2):
            visitor(*visitor), param1(param1), param2(param2)
            {}

        RET operator() (IRValueInvalid &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRValueComment &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRGlobalVar &val) { return visitor.visit(val, param1, param2); }

        RET operator() (IRBuildInTypeAllocation &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRConstNumberI32 &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRConstBoolean &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRNumberCalculationBinary &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRNumberCompareBinary &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRLoad &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRStore &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRReturn &val) { return visitor.visit(val, param1, param2); }

        RET operator() (IRJump &val) { return visitor.visit(val, param1, param2); }
        RET operator() (IRConditionalJump &val) { return visitor.visit(val, param1, param2); }
    };
      */
};



