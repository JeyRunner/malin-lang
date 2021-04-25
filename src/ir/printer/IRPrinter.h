#pragma once
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include "ValueNamesScope.h"
#include <set>

/**
 * Generate Intermediate representation from the AST.
 */
class IRPrinter : public IRVisitor::IRValueVisitor<void, int>
{
  public:
    IRPrinter(ostream &os) : os(os) {
    }

    void print(IRModule &module) {
      os << "IR module (srcFileName: " << module.sourceFileName << "):" << endl << endl;
      for (IRValueVar &var : module.globalVariables) {
        localNames.restNames();
        visitIRValue(var, 0);
        os << endl;
      }
      for (IRFunction &function : module.functions) {
        localNames.restNames();
        visitIRFunction(function);
        os << endl;
      }
    }


  private:
    // used value names global
    map<string, int>  valueNamesLastFunction;
    map<IRValue*, string>valueNamesFunction;

    ValueNamesScope globalNames = ValueNamesScope('@');
    /// within a function
    ValueNamesScope localNames = ValueNamesScope('%');

    // used value names in the current function <- todo
    // map<string, int> valueNamesGlobal;

    ostream &os;

    /**
     * Print name, type and '=' for a new local(created within a function) IRValue.
     */
    ostream& osi(IRValue &value) {
      os << "    ";
      os << localNames.createValueDeclStr(value);
      return os;
    }


    string valStr(IRValueVar* value) {
      //return irTypeToString(value->type) + " " + "%" + valueNamesFunction[(IRValue*) value];
      //auto typeStr = irTypeToString(((IRValue*)value)->type);
      //return typeStr + " %" + valueNamesFunction[(IRValue*) value];
      auto localName = localNames.getValueStr(value);
      if (localName) {
        return localName.value();
      }
      return globalNames.getValueStr(value).value();
    }





    void visitIRFunction(IRFunction &function) {
      if (!function.isExtern) {
        os << "function @" << function.name << "(): " << irTypeToString(function.returnType) << " {" << endl;
        for (IRBasicBlock &bb : function.basicBlocks) {
          visitIRBasicBlock(bb);
          cout << endl;
        }
        os << "}" << endl;
      }
      else {
        os << "function @" << function.name << "()  [extern]" << endl;
      }
    }

    void visitIRBasicBlock(IRBasicBlock &bb) {
      os << "  " << bb.name << ": " << endl;
      for (IRValueVar &value : bb.instructions) {
        visitIRValue(value, 0);

        if (&value != &bb.instructions.back()) {
          os << endl;
        }
      }
    }


  public:
    void visit(IRValueInvalid &val, int param) override {
      cout << "IRValueInvalid";
    }


    void visit(IRGlobalVar &val, int param) override {
      //os << "{" << endl;
      visitIRValue(*val.initValue, 0);
      //os << "@" << val.name << ": " << irTypeToString(val.type) << " = globalVar( " << valStr(val.initValue) << " )" << endl;
      os << globalNames.createValueDeclStr(val) << " globalVar( " << valStr(val.initValue) << " )";
      //os << "}" << endl;
    }

    void visit(IRBuildInTypeAllocation &val, int param) override {
      IRType typeToAlloc = *get<IRTypePointer>(val.type).pointTo;
      osi(val) << "allocBuildIn( " << irTypeToString(typeToAlloc) << " )";
    }

    void visit(IRConstNumberI32 &val, int param) override {
      osi(val) << val.value;
    }

    void visit(IRNumberCalculationBinary &val, int param) override {
      osi(val) << "numberCalculationBinary( " << valStr(val.lhs) << ", " << toString(val.op) << " ," << valStr(val.rhs) << " )";
    }

    void visit(IRLoad &val, int param) override {
      osi(val) << "load( " << valStr(val.valueToLoad) << " )";
    }
    void visit(IRStore &val, int param) override {
      osi(val) << "store( " << valStr(val.valueToStore) << ", " << valStr(val.destinationPointer) << " )";
    }

    void visit(IRReturn &val, int param) override {
      if (val.returnValue == nullptr) {
        osi(val) << "return()";
      }
      else {
        osi(val) << "return( " << valStr(val.returnValue) << " )";
      }
    }
};



