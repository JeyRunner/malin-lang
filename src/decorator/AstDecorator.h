#pragma once


#include <memory>
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
      // add classes to scope
      for (auto &classDecl : root.classDeclarations) {
        if (!globalScope.addName(classDecl->name, *classDecl)) {
          error("name '" + classDecl->name + "' already declared", classDecl->location)
              .printMessage("name '" + classDecl->name + "' previously declared here", globalScope.findName(classDecl->name)->location);
          continue;
        }
      }


      // resolve class member signature
      for (auto &classDecl : root.classDeclarations) {
        doClassDeclarationSignature(classDecl.get());
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

      // resolve class functions body
      for (auto &classDecl : root.classDeclarations) {
        doClassDeclarationBody(classDecl.get());
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
      else if (auto* ex = dynamic_cast<StringExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_str);
        return true;
      }
      else if (auto* ex = dynamic_cast<UnaryExpression*>(expression)) {
        return doUnaryExpression(ex, isolated);
      }
      // variable expression
      else if (auto* ex = dynamic_cast<VariableExpression*>(expression)) {
        if (!isolated) {
          return doVariableExpression(ex, isolated);
        }
        else {
          error("usage of other variables is not allowed here", ex->location);
          return false;
        }
      }
      // call expression
      else if (auto* ex = dynamic_cast<CallExpression*>(expression)) {
        if (!isolated) {
          return doCallExpression(ex, isolated);
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
    bool doUnaryExpression(UnaryExpression *ex, bool isolated) {
      auto innerOk = doExpression(ex->innerExpression.get(), isolated);
      if (!innerOk) {
        return false;
      }
      // check type
      if (ex->operation == Expr_Unary_Op_LOGIC_NOT) {
        ex->resultType = make_unique<BuildInType>(BuildIn_bool);
        // inner is not bool
        if (!ex->innerExpression->resultType->equals(boolType.get())) {
          error("type '"+ ex->innerExpression->resultType->toString() +"' of inner expression is not required type '"+boolType->toString()+"' for unary expression '"
                    + string(magic_enum::enum_name(ex->operation))+ "'",
                ex->location);
        }
      }
      else {
        error("only not unary expression is currently supported but not '"
                  + string(magic_enum::enum_name(ex->operation))+ "'",
              ex->location);
        return false;
      }
      return true;
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


    /**
     * @param isolated true if expression is part of assigment of a global variable
     *                 and no variable or call expressions are allowed
     * @return false if there is a error in expression, this will prevent continuing of checking
     */
    bool doVariableExpression(VariableExpression *ex, bool isolated){
      // if is member var: used from outside of the class like 'myObject.myVar'
      if (auto* memberVar = dynamic_cast<MemberVariableExpression*>(ex)) {
        if (!doExpression(memberVar->parent.get(), isolated))
          return false;
        auto parentClassType= dynamic_cast<ClassType*>(memberVar->parent->resultType.get());
        if (!parentClassType) {
          error("type '" + memberVar->parent->resultType->toString() + "' is not a class type and can't have members, thus member '"+ex->name+"' was not found",
                memberVar->parent->location);
          return false;
        }
        // find member
        auto memberDecl = parentClassType->classDeclaration->findMemberVariable(ex->name);
        if (!memberDecl) {
          error("class '" + parentClassType->classDeclaration->name + "' has no member with name '"+ex->name+"'", ex->location)
            .printMessage("class '" + parentClassType->classDeclaration->name + "' defined here", parentClassType->classDeclaration->location);
          return false;
        }
        ex->variableDeclaration = memberDecl;
        ex->resultType = memberDecl->type->clone();
        return true;
      }
      // not a member var
      else {
        auto node = namesStack.findName(ex->name);
        if (!node) {
          error("name '" + ex->name + "' not found in current scope", ex->location);
          return false;
        }
        auto varDecl = dynamic_cast<AbstractVariableDeclaration *>(node);
        if (!varDecl) {
          error("'" + ex->name + "' is not a declared Variable", ex->location);
          return false;
        }
        // check if var is member of parent class
        if (varDecl->isMemberVariable()) {
          // create MemberVariableExpression that points to this of class
          // and replace ex with it
          unique_ptr<MemberVariableExpression> memberExpr = make_unique<MemberVariableExpression>();
          memberExpr->location = ex->location;
          memberExpr->variableDeclaration = ex->variableDeclaration;
          memberExpr->name = ex->name;
          memberExpr->parentAstNode = ex->parentAstNode;
          // parent is this
          auto thisParent = make_unique<VariableExpression>();
          thisParent->variableDeclaration = varDecl->parentClass->thisVarDecl.get();
          thisParent->name = "this";
          thisParent->location = ex->location;
          thisParent->resultType = thisParent->variableDeclaration->type->clone();
          thisParent->parentAstNode = memberExpr.get();
          memberExpr->parent = move(thisParent);

          // replace ex with memberExpr
          // this will delete the old ex
          // fix ex pointer, now points to memberExpr
          ex = ex->replaceNode(move(memberExpr));
        }

        ex->variableDeclaration = varDecl;
        if (!varDecl->type) {
          return false;
        }
        ex->resultType = varDecl->type->clone();
        return true;
      }
    }


    /**
     * @param isolated true if expression is part of assigment of a global variable
     *                 and no variable or call expressions are allowed
     * @return false if there is a error in expression, this will prevent continuing of checking
     */
    bool doCallExpression(CallExpression *call, bool isolated) {
      FunctionDeclaration* func = resolveFunctionDeclOfCall(call, isolated);
      if (!func) {
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
        arg.argName = nullopt; // remove the name
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
          newArg.expression = defaultExprConst->clone();
          newArg.location = call->location;
          newArg.argumentDeclaration = &func->arguments[i];
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


    /**
     * Finds FunctionDeclaration of the called function from a CallExpression.
     * Prints error if not found.
     * @return the FunctionDeclaration if found, nullptr otherwise
     */
    FunctionDeclaration *resolveFunctionDeclOfCall(CallExpression *call, bool isolated) {
      FunctionDeclaration* func = nullptr;
      // if is member call
      if (auto* memberCall = dynamic_cast<MemberCallExpression*>(call)) {
        if (!doExpression(memberCall->parent.get(), isolated))
          return nullptr;
        auto parentClassType= dynamic_cast<ClassType*>(memberCall->parent->resultType.get());
        if (!parentClassType) {
          error("type '" + memberCall->parent->resultType->toString() + "' is not a class type and can't have members, thus member function '"+call->calledName+"' was not found",
              memberCall->parent->location);
          return nullptr;
        }
        // find member
        auto memberDecl = parentClassType->classDeclaration->findMemberFunction(call->calledName);
        if (!memberDecl) {
          error("class '" + parentClassType->classDeclaration->name + "' has no member function with name '"+call->calledName+"'", call->location)
              .printMessage("class '" + parentClassType->classDeclaration->name + "' defined here", parentClassType->classDeclaration->location);
          return nullptr;
        }
        func = memberDecl;
      }
      // NOT a member call
      else {
        auto foundNode = namesStack.findName(call->calledName);
        func = dynamic_cast<FunctionDeclaration *>(foundNode);
        // check if its a constructor
        if (auto classDecl = dynamic_cast<ClassDeclaration *>(foundNode))
        {
          func = classDecl->constructor.get();
        }
      }
      if (!func){
        error("function with name '" + call->calledName + "' not declared", call->location);
      }
      return func;
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
     * @return true when statement is a return statement or it contains a return statement
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
      // while statement
      else if (auto* st = dynamic_cast<WhileStatement*>(statement)) {
        return doWhileStatement(st, expectedTypeForReturn);
      }
      // variable assign statement
      else if (auto* st = dynamic_cast<VariableAssignStatement*>(statement)) {
        doVariableAssignStatement(st);
        return false;
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


    bool doWhileStatement(WhileStatement *st, LangType *expectedTypeForReturn) {
      if (doExpression(st->condition.get(), false)) {
        if (!st->condition->resultType->equals(boolType.get())) {
          error("condition of if has to be of type bool, but is '"+st->condition->resultType->toString()+"'",
                st->condition->location);
        }
      }

      doCompoundStatementWithNewScope(st->body.get(), expectedTypeForReturn);
      // condition could jump over the body if its initial false
      return false;
    }


    void doVariableAssignStatement(VariableAssignStatement *st) {
      if (!doVariableExpression(st->variableExpression.get(), false))
        return;
      if (!st->variableExpression->variableDeclaration->isMutable) {
        error("can't assign a value to a non mutable variable '"
                    + st->variableExpression->variableDeclaration->name + "'",
              st->location);
      }

      if (!doExpression(st->valueExpression.get(), false))
        return;
      // check types
      auto varType = st->variableExpression->resultType.get();
      auto valType = st->valueExpression->resultType.get();
      if (!varType->equals(valType)) {
        error("operand types of variable assignment are not the same: variable type '"
                  + varType->toString() + "' and value type '"
                  + valType->toString() + "'",
              st->location);
      }
    }



    /**
     * This will check member variables (type and init) and member functions (signature with args and types).
     * This will not do the member func body.
     */
    void doClassDeclarationSignature(ClassDeclaration *classDecl) {
      // resolve functions return type and argument types
      for (auto &node : classDecl->functionDeclarations) {
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
      // add this arg
      classDecl->thisVarDecl = make_unique<VariableDeclaration>();
      classDecl->thisVarDecl->name = "this";
      classDecl->thisVarDecl->location = classDecl->location;
      // @todo this should be ReferenceType<ClassType>
      classDecl->thisVarDecl->type = make_unique<ClassType>(classDecl);

      // add default constructor
      classDecl->constructor->name = classDecl->name + "_default_constructor";
      classDecl->constructor->parentClass = classDecl;
      classDecl->constructor->isConstructor = true;
      classDecl->constructor->returnType = make_unique<ClassType>(classDecl);

      // resolve vars type and init expressions
      for (auto &varDecl : classDecl->variableDeclarations) {
        doVariableDeclaration(varDecl.get(), true, false);
      }
    }


    /**
     * This will check member functions bodies.
     * This will NOT check member variables (type and init) and member functions (signature with args and types).
     */
    void doClassDeclarationBody(ClassDeclaration *classDecl) {
      // new class scope
      // add member vars and functions to that scope
      NamesScope &classScope = namesStack.addNamesScope();
      for (auto &varDecl : classDecl->variableDeclarations) {
        addNameToScope(classScope, varDecl->name, *varDecl);
      }
      // add this var to scope
      addNameToScope(classScope, "this", *classDecl->thisVarDecl);
      // func signature
      for (auto &funcDecl : classDecl->functionDeclarations) {
        addNameToScope(classScope, funcDecl.name, funcDecl);
      }
      // functions body
      for (auto &funcDecl : classDecl->functionDeclarations) {
        doFunctionDeclarationBody(&funcDecl);
      }
      namesStack.removeNamesScope(classScope);
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
    void doVariableDeclaration(VariableDeclaration *varDecl, bool constInit, bool needsInit = true) {
      auto initExpr = varDecl->initExpression.get();
      if (!initExpr && needsInit) {
        error("variable needs an init expression but got none",varDecl->location);
        return;
      }
      // check init expr
      LangType *initExprType;
      if (initExpr) {
        if (constInit) {
          if (!dynamic_cast<ConstValueExpression*>(varDecl->initExpression.get())) {
            error("global variable need to have a constant init expression, this expression is not constant",
                  varDecl->initExpression->location);
            return;
          }
        }

        // resolve initExpression
        bool initOk = doExpression(varDecl->initExpression.get(), constInit);
        initExprType = varDecl->initExpression->resultType.get();
        if (!initOk || !initExprType)
          return;
      }


      // when explicit typename given
      if (varDecl->typeName.length() > 0) {
        varDecl->type = makeTypeForName(varDecl->typeName, varDecl->location);
        // check type of init
        if (initExpr) {
          if (!initExprType->equals(varDecl->type.get())) {
            error("specified type of variable '"+ varDecl->type->toString() +"' "
                      +"dose not not matches type of init expression '"+ initExprType->toString() +"'", varDecl->location);
          }
        }
      }
      // infer type by init expression
      else if (initExpr) {
        varDecl->type = initExprType->clone();
      }
      else {
        error("variable '"+varDecl->name+"' needs a type, but no explicit type was provided or could be inferred from a init expression", varDecl->location);
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
      if (typeName == "str")
        return BuildIn_str;
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
      // user defined type
      else {
        auto found = namesStack.findName(name);
        if (!found) {
          error("could not find type with name '" + name+ "'", location);
          return nullptr;
        }
        auto classDecl = dynamic_cast<ClassDeclaration*>(found);
        if (!classDecl) {
          error("name '" + name+ "' is not a user defined type like a class", location);
          return nullptr;
        }
        return make_unique<ClassType>(classDecl);
      }
    }


    /**
     * Adds a name to the provided scope and prints an error message when name already exists
     * @return true if name not already exists
     */
    bool addNameToScope(NamesScope &scope, string name, ASTNode &node) {
      if (!scope.addName(name, node)) {
        error("name '" + name + "' already declared", node.location)
            .printMessage("name '" + name + "' previously declared here", scope.findName(name)->location);
        return false;
      }
      return true;
    }


    template <class T>
    T findNameAs(string name) {
      auto node = namesStack.findName(std::move(name));
      if (!node) {

      }
      throw runtime_error("findNameAs not implemented");
    }
};