#pragma once


#include <utility>
#include "../parser/AST.h"
#include "../Log.h"
#include "NamesStack.h"
#include "BinaryOpSupportedTypes.h"

using namespace std;



class AstDecorator {
  public:
    bool linkNames(RootDeclarations &root) {
      // add top names scope for global names
      NamesScope &globalScope = namesStack.addNamesScope();

      // add vars to scope
      for (auto &var : root.variableDeclarations) {
        if (!globalScope.addName(var->name, *var)) {
          error("name '" + var->name + "' already declared", var->location)
            .printMessage("name '" + var->name + "' previously declared here", globalScope.findName(var->name)->location);
        }
      }
      // add functions to scope
      for (auto &func : root.functionDeclarations) {
        if (!globalScope.addName(func.name, func)) {
          error("name '" + func.name + "' already declared", func.location)
            .printMessage("name '" + func.name + "' previously declared here", globalScope.findName(func.name)->location);
          continue;
        }

      }


      // resolve functions return type and argument types
      for (auto &node : root.functionDeclarations) {
        node.returnType = makeTypeForName(node.typeName, node.location);
        // arguments
        for (auto &arg : node.arguments) {
          arg.type = makeTypeForName(arg.typeName, arg.location);
          // default expression
          if (arg.defaultExpression) {
            doExpression(arg.defaultExpression.get(), true);
          }
        }
      }

      // resolve vars type and init expressions
      for (auto &varDecl : root.variableDeclarations) {
        doVariableDeclaration(varDecl.get(), true);
      }

      // resolve functions body
      // and search for main function
      for (auto &func : root.functionDeclarations) {
        bool isMain = doFunctionDeclarationBody(&func);
        if (isMain) {
          root.mainFunction = &func;
        }
      }


      // if no main
      if (!root.mainFunction) {
        ::error("no main function has been provided, the main function needs the signature 'func main(): i32'");
        errors++;
      }

        // return
      return errors == 0;
    }




  private:
    /**
     * @param isolated true if expression is part of assigment of a global variable
     *                 and no variable or call expressions are allowed
     * @return false if there is a error in expression, this will prevent continuing of checking
     */
    bool doExpression(Expression *expression, bool isolated) {
      // check type of expression
      if (auto* ex = dynamic_cast<NumberIntExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_i32);
        return true;
      }
      else if (auto* ex = dynamic_cast<NumberFloatExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_f32);
        return true;
      }
      else if (auto* ex = dynamic_cast<BoolExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_bool);
        return true;
      }
      // variable expression
      else if (auto* ex = dynamic_cast<VariableExpression*>(expression)) {
        if (!isolated) {
          return doVariableExpression(ex);
        }
        else {
          error("usage of other variables is not allowed here", ex->location);
          return false;
        }
      }
      // call expression
      else if (auto* ex = dynamic_cast<CallExpression*>(expression)) {
        if (!isolated) {
          return doCallExpression(ex);
        }
        else {
          error("usage of function calls is not allowed here", ex->location);
          return false;
        }
      }
      // binary expression
      else if (auto* ex = dynamic_cast<BinaryExpression*>(expression)) {
        return doBinaryExpression(ex, isolated);
      }

      error("unsupported expression ", expression->location);
      return false;
    }


    /**
     * @param isolated true if expression is part of assigment of a global variable
     *                 and no variable or call expressions are allowed
     * @return false if there is a error in expression, this will prevent continuing of checking
     */
    bool doBinaryExpression(BinaryExpression *ex, bool isolated) {
      bool lhsOk = doExpression(ex->lhs.get(), isolated);
      bool rhsOk = doExpression(ex->rhs.get(), isolated);
      if (!lhsOk || !rhsOk){
        return false;
      }
      // compare types of lhs and rhs
      auto lhsType = ex->lhs->resultType.get();
      auto rhsType = ex->rhs->resultType.get();
      if (lhsType->equals(rhsType)) {
        // check result type
        ex->resultType = binaryOperationResultType(lhsType, ex->operation);
        if (ex->resultType->isInvalid()) {
          error("type '"+ ex->lhs->resultType->toString() +"' does not support binary expression '"
                    + string(magic_enum::enum_name(ex->operation))+ "'",
                ex->location);
          return false;
        }
      }
      else {
        error("operand types of binary expression are not the same: lhs type '"
                  + lhsType->toString() + "' and rhs type '"
                  + rhsType->toString() + "'",
              ex->location);
        return false;
      }

      return true;
    }


    bool doVariableExpression(VariableExpression *ex){
      auto node = namesStack.findName(ex->name);
      if (!node) {
        error("name '"+ ex->name +"' not found in current scope", ex->location);
        return false;
      }
      auto varDecl = dynamic_cast<AbstractVariableDeclaration*>(node);
      if (!varDecl) {
        error("'"+ ex->name +"' is not a declared Variable", ex->location);
        return false;
      }

      ex->variableDeclaration = varDecl;
      if (!varDecl->type) {
        return false;
      }
      ex->resultType = varDecl->type->clone();
      return true;
    }


    bool doCallExpression(CallExpression *call) {
      auto func = dynamic_cast<FunctionDeclaration*>(namesStack.findName(call->calledName));
      if (!func) {
        error("function with name '"+ call->calledName +"' not declared", call->location);
        return false;
      }
      if (!func->returnType) {
        //return false;
      }

      bool ok = true;

      // create empty args vector
      // index is the real index of the argument
      // if an index is nullptr no arg was provided
      vector<CallExpressionArgument> callArgs(func->arguments.size());
      vector<bool> callArgsValid(func->arguments.size(), false);

      // non named args
      auto argIndex = 0;
      for (auto &arg : call->argumentsNonNamed) {
        if (argIndex >= func->arguments.size()){
          error("function '" + func->name + "' has only " + to_string(func->arguments.size()) + " arguments, "
                    + "but a " + to_string(argIndex + 1) + ". argument has been provided at the function call",
                arg.location);
          ok = false;
          break;
        }

        // check type of expression
        ok&= checkFunctionCallArgType(func, arg, argIndex);
        // link decl and save in callArgs
        arg.argumentDeclaration = &func->arguments.at(argIndex);
        callArgs[argIndex] = move(arg);
        callArgsValid[argIndex] = true;
        argIndex++;
      }

      // named args
      for (auto &arg : call->argumentsNamed) {
        // get arg index from function declaration
        auto namedArgAt = find_if(func->arguments.begin(), func->arguments.end(), [&](FunctionParamDeclaration &argDecl) {
          return argDecl.name == arg.argName;
        }) - func->arguments.begin();

        // name not found
        if (namedArgAt >= func->arguments.size()) {
          error("function '" + func->name + "' dose not have a argument with name '"+*arg.argName+"'",
                arg.location);
          ok = false;
          continue;
        }

        // arg already assigned
        if (callArgsValid.at(namedArgAt)) {
          error("function argument '"+*arg.argName+"' of function '" + func->name + "' was already assigned by an other argument before",
                arg.location)
                .printMessage("first assign of argument '"+*arg.argName+"'", callArgs.at(namedArgAt).location);
          ok = false;
          continue;
        }

        // check type of expression
        ok&= checkFunctionCallArgType(func, arg, namedArgAt);
        // link decl and save in callArgs
        arg.argumentDeclaration = &func->arguments.at(namedArgAt);
        callArgs[namedArgAt] = move(arg);
        callArgsValid[namedArgAt] = true;
      }


      // set default values for unassigned
      for (int i = 0; i < callArgs.size(); i++) {
        if (callArgsValid[i]) {
          continue;
        }

        // check if arg has default value
        Expression *defaultExpr = func->arguments[i].defaultExpression.get();
        if (!defaultExpr) {
          error("function argument '"+func->arguments[i].name+"' of function '" + func->name + "' is required but has not been provided at function call",
                call->location)
                .printMessage("definition of argument '"+ func->arguments[i].name +"'", func->arguments[i].location);
          ok = false;
        }
        // has default expr
        else {
          CallExpressionArgument newArg = CallExpressionArgument();
          auto defaultExprConst = dynamic_cast<ConstValueExpression*>(defaultExpr);
          if (!defaultExprConst) {
            error("only const values are supported for default function arguments", defaultExpr->location);
            continue;
          }

          // expression now shared with function declaration argument
          newArg.expression = func->arguments[i].defaultExpression;
          newArg.location = call->location;
          callArgs[i] = move(newArg);
        }
      }
      // set computed args
      call->argumentsNonNamed = move(callArgs);
      call->argumentsNamed.resize(0);

      // aboard when error occurred
      if (!ok) {
        return false;
      }


      // return type
      if (!func->returnType) {
        return false;
      }
      else {
        call->resultType = func->returnType->clone();
        call->functionDeclaration = func;
      }

      return true;
    }


    bool checkFunctionCallArgType(const FunctionDeclaration *func,
                                  const CallExpressionArgument &arg,
                                  int callArgIndex) {
      // check type of expression
      if (doExpression(arg.expression.get(), false)) {
        auto funcArgType = func->arguments.at(callArgIndex).type.get();
        if (!arg.expression->resultType->equals(funcArgType)) {
          error("function argument '"+*arg.argName+"' of function '" + func->name + "' needs type '"+ funcArgType->toString() +"' "+
                "but assigned expression has type '"+ arg.expression->resultType->toString() +"'",
                arg.expression->location);
          return false;
        }
      } else {
        return false;
      }
      return true;
    }





    /*************************************************************************
     **** Statements *********************************************************
     */


    /**
     * @return true when statment is a return statement or it contains a return statement
     */
    bool doStatement(Statement *statement, NamesScope &scope, LangType *expectedTypeForReturn) {
      // return
      if (auto* st = dynamic_cast<ReturnStatement*>(statement)) {
        doReturnStatement(st, expectedTypeForReturn);
        return true;
      }
      // variable declaration
      else if (auto* st = dynamic_cast<VariableDeclaration*>(statement)) {
        doVariableDeclaration(st, false);
        if (!scope.addName(st->name, *st)) {
          error("name '" + st->name + "' already declared", st->location)
              .printMessage("name '" + st->name + "' previously declared here", scope.findName(st->name)->location);
        }
        return false;
      }
      // compound statement
      else if (auto* st = dynamic_cast<CompoundStatement*>(statement)) {
        return doCompoundStatementWithNewScope(st, expectedTypeForReturn);
      }
      // if statement
      else if (auto* st = dynamic_cast<IfStatement*>(statement)) {
        return doIfStatement(st, expectedTypeForReturn);
      }
      // expression statement
      else if (auto* st = dynamic_cast<Expression*>(statement)) {
        doExpression(st, false);
        return false;
      }
      else {
        error("unsupported statement", statement->location);
        return false;
      }
    }


    void doReturnStatement(ReturnStatement *st, LangType *expectedTypeForReturn) {
      // is void
      if (!st->expression) {
        st->returnType = make_unique<BuildInType>(BuildIn_void);
      }
        // is non void
      else {
        bool exprOk = doExpression(st->expression->get(), false);
        if (!exprOk)
          return;
        st->returnType = st->expression->get()->resultType->clone();
      }
      // type check
      if (!st->returnType->equals(expectedTypeForReturn)) {
        string exprType = st->returnType->toString();
        error("expected return type '"+ expectedTypeForReturn->toString() +"' for function "
                  +"does not match given return type '"+ exprType +"'", st->location);
      }
    }


    /**
     * This will not create a new scope, but will use the given scope.
     * @return true if it contains a return statement
     */
    bool doCompoundStatement(CompoundStatement *st, NamesScope &scope, LangType *expectedTypeForReturn) {
      bool hasReturn = false;
      for (auto &statement : st->statements) {
        if (hasReturn) {
          printWarn("", "statement is after return and will be ignored", statement->location);
        }
        bool currentHasReturn = doStatement(statement.get(), scope, expectedTypeForReturn);
        hasReturn = hasReturn || currentHasReturn;
      }
      return hasReturn;
    }

    /**
     * This will create a new scope.
     * @return true if it contains a return statement
     */
    bool doCompoundStatementWithNewScope(CompoundStatement *st, LangType *expectedTypeForReturn) {
      NamesScope &compScope = namesStack.addNamesScope();
      bool hasReturn = doCompoundStatement(st, compScope, expectedTypeForReturn);
      namesStack.removeNamesScope(compScope);
      return hasReturn;
    }


    bool doIfStatement(IfStatement *st, LangType *expectedTypeForReturn) {
      if (doExpression(st->condition.get(), false)) {
        if (!st->condition->resultType->equals(boolType.get())) {
          error("condition of if has to be of type bool, but is '"+st->condition->resultType->toString()+"'",
              st->condition->location);
        }
      }

      bool hasReturn = doCompoundStatementWithNewScope(st->ifBody.get(), expectedTypeForReturn);
      if (st->elseBody) {
        bool elseHasReturn = doCompoundStatementWithNewScope(st->elseBody.get(), expectedTypeForReturn);
        hasReturn = hasReturn && elseHasReturn;
      }
      return hasReturn;
    }



    /**
     * This will not do the func arguments and return type, this has to be setup before.
     * @return true if func is the main function otherwise false
     */
    bool doFunctionDeclarationBody(FunctionDeclaration *func) {
      // skip if no valid return type
      if (!func->returnType) {
        return false;
      }

      bool isMain = false;
      // new function scope
      // add arguments to that scope
      NamesScope &funcScope = namesStack.addNamesScope();
      for (auto &arg : func->arguments) {
        funcScope.addName(arg.name, arg);
      }

      // body
      if (func->body) {
        bool hasReturn = doCompoundStatement(func->body.get(), funcScope, func->returnType.get());
        // check return
        if (!hasReturn) {
          if (func->returnType->isVoidType()) {
            // insert implicit return void
            auto ret = make_unique<ReturnStatement>();
            ret->returnType = make_unique<BuildInType>(BuildIn_void);
            func->body->statements.push_back(move(ret));
          }
          else {
            error("a non void function has to return something at the end", func->location);
          }
        }
      }

      // check for main function
      if (func->name == "main") {
        if (func->arguments.empty() && func->returnType->equals(requiredMainReturnType.get())) {
          isMain = true;
        }
        else {
          error("main function has wrong signature, it needs the signature 'func main(): i32'", func->location);
        }
      }

      namesStack.removeNamesScope(funcScope);
      return isMain;
    }



    /**
     * Will not add var to namesScope.
     */
    void doVariableDeclaration(VariableDeclaration *varDecl, bool constInit) {
      if (constInit) {
        if (!dynamic_cast<ConstValueExpression*>(varDecl->initExpression.get())) {
          error("global variable need to have a constant init expression, this expression is not constant",
              varDecl->initExpression->location);
          return;
        }
      }

      // resolve initExpression
      bool initOk = doExpression(varDecl->initExpression.get(), constInit);
      LangType *initExprType = varDecl->initExpression->resultType.get();
      if (!initOk || !initExprType)
        return;

      // when explicit typename given
      if (varDecl->typeName.length() > 0) {
        varDecl->type = makeTypeForName(varDecl->typeName, varDecl->location);
        if (!initExprType->equals(varDecl->type.get())) {
          error("specified type of variable '"+ varDecl->type->toString() +"' "
                    +"dose not not matches type of init expression '"+ initExprType->toString() +"'", varDecl->location);
        }
      }
        // infer type by init expression
      else {
        varDecl->type = initExprType->clone();
      }
    }




  private:
    NamesStack namesStack;
    int errors = 0;
    unique_ptr<BuildInType> requiredMainReturnType = make_unique<BuildInType>(BuildIn_i32);
    unique_ptr<BuildInType> boolType = make_unique<BuildInType>(BuildIn_bool);

    MsgScope error(
        const string& msg,
        SrcLocationRange &location)
    {
      errors++;
      return printError("decorating", msg, location);
    }


    static BUILD_IN_TYPE typeNameToBuildIn(string typeName) {
      if (typeName == "i32")
        return BuildIn_i32;
      if (typeName == "f32")
        return BuildIn_f32;
      if (typeName == "void")
        return BuildIn_void;
      if (typeName == "bool")
        return BuildIn_bool;
      return BuildIn_No_BuildIn;
    }

    /**
     * make type
     * prints error and returns null if type not found.
     */
    unique_ptr<LangType> makeTypeForName(string &name, SrcLocationRange &location) {
      BUILD_IN_TYPE buildIn = typeNameToBuildIn(name);
      if (buildIn != BuildIn_No_BuildIn)
      {
        auto type = make_unique<BuildInType>(buildIn);
        return move(type);
      }
      else {
        error("only BuildIn types are currently supported, '" + name+ "' is not a BuildIn type", location);
        return nullptr;
      }
    }

    template <class T>
    T findNameAs(string name) {
      auto node = namesStack.findName(std::move(name));
      if (!node) {

      }
      throw runtime_error("findNameAs not implemented");
    }
};