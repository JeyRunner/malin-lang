#pragma once
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include "ValueNamesScope.h"
#include <ranges>
#include <set>


/**
 * Generate Intermediate representation from the AST.
 */
class IRPrinter : private IRVisitor::IRValueVisitor<void, int>
{
  public:
    IRPrinter(ostream &os) : os(os) {
    }

    void print(IRModule &module) {
      os << "IR module (srcFileName: " << module.sourceFileName << "):" << endl << endl;
      for (IRValueVar &var : module.globalVariables) {
        localNames.restNames();
        visitIRValue(var, 0);
        os << endl << endl;
      }
      os << endl;
      for (IRFunction &function : module.functions) {
        localNames.restNames();
        functionBBNames.restNames();
        visitIRFunction(function);
        os << endl << endl;
      }
    }


  private:
    ValueNamesScope globalNames = ValueNamesScope('@');
    /// within a function
    ValueNamesScope localNames = ValueNamesScope('%');
    IRNamesScope functionBBNames;

    ostream &os;

    /**
     * Print name, type and '=' for a new local(created within a function) IRValue.
     */
    ostream& osi(IRValue &value) {
      os << "    ";
      os << localNames.createValueDeclStr(value);
      return os;
    }


    /**
     * Get name of ir value.
     */
    string valStr(IRValueVar* value) {
      //return irTypeToString(value->type) + " " + "%" + valueNamesFunction[(IRValue*) value];
      //auto typeStr = irTypeToString(((IRValue*)value)->type);
      //return typeStr + " %" + valueNamesFunction[(IRValue*) value];
      auto localName = localNames.getValueStr(value);
      if (localName) {
        return localName.value();
      }
      auto globalName = globalNames.getValueStr(value);
      if (globalName) {
        return globalName.value();
      }
      return "?UNKOWN?";
    }


    /**
     * Get name of basic block.
     */
    string bbStr(IRBasicBlock *bb) {
      return "bb " + functionBBNames.getName(bb);
    }





    void visitIRFunction(IRFunction &function) {
      // default irValues for the function arguments
      //std::count_if()
      int argsWithInitValue = ranges::count_if(function.arguments, [](auto arg){return ((IRFunctionArgument &)arg).initValue;});

      if (!function.arguments.empty() && argsWithInitValue > 0) {
        os << "{" << endl;
        for (auto &arg_ : function.arguments) {
          auto arg = (IRFunctionArgument &) arg_;
          if (arg.initValue != nullptr) {
            visitIRValue(*arg.initValue, 0);
            cout << endl;
          }
        }
        os << "}" << endl;
      }

      os << "function @" << function.name << "(";
      // args
      int argIndex = 0;
      for (auto &arg : function.arguments) {
        visitIRValue(arg, 0);
        if (argIndex < function.arguments.size() - 1) {
          os << ", ";
        }
        argIndex++;
      }
      os << "): " << irTypeToString(function.returnType) + " ";

      if (!function.isExtern) {
        os << "{" << endl;
        for (IRBasicBlock &bb : function.basicBlocks) {
          visitIRBasicBlock(bb);
          cout << endl;
        }
        os << "}" << endl;
      }
      else {
        os << "[extern]" << endl;
      }
    }


    void visit(IRFunctionArgument &arg, int param) override {
      os << localNames.createValueDeclStr(arg, false);
      if (arg.initValue != nullptr) {
        os << " = defaultArgValue( " << valStr(arg.initValue) << " )";
      }
    }



    void visitIRBasicBlock(IRBasicBlock &bb) {
      os << " " << functionBBNames.getName(&bb) << ": " << endl;
      for (IRValueVar &value : bb.instructions) {
        visitIRValue(value, 0);

        if (&value != &bb.instructions.back()) {
          os << endl;
        }
      }
    }


    void visit(IRValueInvalid &val, int param) override {
      os << "IRValueInvalid";
    }

    void visit(IRValueComment &val, int param) override {
      // always inside a basic block therefore indented
      osi(val) << "// " << val.comment;
    }


    void visit(IRGlobalVar &val, int param) override {
      os << "{";
      visitIRValue(*val.initValue, 0);
      //os << "@" << val.name << ": " << irTypeToString(val.type) << " = globalVar( " << valStr(val.initValue) << " )" << endl;
      os << "    }" << endl;
      os << globalNames.createValueDeclStr(val) << " globalVar( " << valStr(val.initValue) << " )";
    }

    void visit(IRBuildInTypeAllocation &val, int param) override {
      IRType typeToAlloc = *get<IRTypePointer>(val.type).pointTo;
      osi(val) << "allocBuildIn( " << irTypeToString(typeToAlloc) << " )";
    }

    void visit(IRConstNumberI32 &val, int param) override {
      osi(val) << val.value;
    }

    void visit(IRConstNumberF32 &val, int param) override {
      osi(val) << val.value;
    }

    void visit(IRConstBoolean &val, int param) override {
      osi(val) << (val.value ? "true" : "false");
    }

    void visit(IRLogicalNot &val, int param) override {
      osi(val) << "not( " << valStr(val.negateValue) << " )";
    }

    void visit(IRNumberCalculationBinary &val, int param) override {
      osi(val) << "numberCalculationBinary( " << valStr(val.lhs) << ", " << toString(val.op) << " ," << valStr(val.rhs) << " )";
    }

    void visit(IRNumberCompareBinary &val, int param) override {
      osi(val) << "numberCompareBinary( " << valStr(val.lhs) << ", " << toString(val.op) << " ," << valStr(val.rhs) << " )";
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



    void visit(IRJump &val, int param) override {
      osi(val) << "jump( " << bbStr(val.jumpToBB) << " )";
    }

    void visit(IRConditionalJump &val, int param) override {
      osi(val) << "jump( " << valStr(val.conditionValue) << ", "
               << "whenTrue -> " << bbStr(val.jumpToWhenTrueBB) << ", "
               << "whenFalse -> " << bbStr(val.jumpToWhenFalseBB) << " )";
    }


    void visit(IRCall &call, int param) override {
      osi(call) << "call( @" << call.function->name;
      // args
      if (!call.function->arguments.empty()) {
        os << ", ";
        int argIndex = 0;
        for (auto *arg: call.arguments) {
          if (arg == nullptr) {
            os << "??"; // currently not set
          }
          else {
            os << valStr(arg);
          }
          if (argIndex < call.arguments.size() - 1) {
            os << ", ";
          }
          argIndex++;
        }
      }
      os << " )";
    }
};



