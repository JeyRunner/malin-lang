#pragma once

#include "../parser/AST.h"
#include "../Log.h"
#include <stack>
#include <map>
#include <utility>
using namespace std;

class AstLink {
  public:
    ASTNode *node;
};
class NamesScope {
  public:
    /**
     * insert new name
     * @return true when insert successful and false when name already exists
     */
    bool addName(const string& name, ASTNode &node) {
      if (findName(name) == nullptr) {
        namesMap.insert({name, AstLink{&node}});
        return true;
      }
      else {
        return false;
      }
    }

    ASTNode * findName(string name) {
      auto it = namesMap.find(name);
      if (it != namesMap.end()) {
        return it->second.node;
      }
      else {
        return nullptr;
      }
    }

  private:
    map<string, AstLink> namesMap;
};

class NamesStack {
  public:
    NamesScope& addNamesScope() {
      return scopes.emplace_back(NamesScope());
    }

    ASTNode * findName(string name) {
      for (auto &it : scopes) {
        ASTNode * node = it.findName(name);
        if (node != nullptr) {
          return node;
        }
      }
      return nullptr;
    }

  private:
    list<NamesScope> scopes;
};


class AstDecorator {
  public:
    bool linkNames(RootDeclarations &root) {
      // add top names scope for global names
      NamesScope &globalScope = namesStack.addNamesScope();

      // add vars and functions to scope
      for (auto &var : root.variableDeclarations) {
        if (!globalScope.addName(var->name, *var)) {
          error("name '" + var->name + "' already declared", var->location)
            .printMessage("name '" + var->name + "' previously declared here", globalScope.findName(var->name)->location);
        }
      }
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
            doExpression(arg.defaultExpression.get(), false);
          }
        }
      }

      // resolve vars type and init expressions
      for (auto &varDecl : root.variableDeclarations) {
        doVariableDeclaration(varDecl.get(), false);
      }

      // resolve functions body
      // and search for main function
      for (auto &func : root.functionDeclarations) {
        // skip if no valid return type
        if (!func.returnType) {
          continue;
        }

        // new function scope
        // add arguments to that scope
        NamesScope &funcScope = namesStack.addNamesScope();
        for (auto &arg : func.arguments) {
          funcScope.addName(arg.name, arg);
        }
        // body
        for (auto &statement : func.bodyStatements) {
          doStatement(statement.get(), funcScope, func.returnType.get());
        }

        // check for main function
        if (func.name == "main") {
          if (func.arguments.empty() && func.returnType->equals(requiredMainReturnType.get())) {
            root.mainFunction = &func;
          }else {
            error("main function has wrong signature, it needs the signature 'func main(): i32'", func.location);
          }
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
     * @param variableExpressionsAllowed false if expression is part of assigment of a global variable
     * @return false if there is a error in expression, this will prevent continuing of checking
     */
    bool doExpression(Expression *expression, bool variableExpressionsAllowed) {
      // check type of expression
      if (auto* ex = dynamic_cast<NumberIntExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_i32);
        return true;
      }
      else if (auto* ex = dynamic_cast<NumberFloatExpression*>(expression)) {
        ex->resultType = make_unique<BuildInType>(BuildIn_f32);
        return true;
      }
      // variable expression
      else if (auto* ex = dynamic_cast<VariableExpression*>(expression)) {
        if (variableExpressionsAllowed) {
          return doVariableExpression(ex);
        }
        else {
          error("usage of other variables is not allowed here", ex->location);
          return false;
        }
      }
      // call expression
      else if (auto* ex = dynamic_cast<CallExpression*>(expression)) {
        return doCallExpression(ex, variableExpressionsAllowed);
      }
      // binary expression
      else if (auto* ex = dynamic_cast<BinaryExpression*>(expression)) {
        bool lhsOk = doExpression(ex->lhs.get(), variableExpressionsAllowed);
        bool rhsOk = doExpression(ex->rhs.get(), variableExpressionsAllowed);
        if (!lhsOk || !rhsOk){
          return false;
        }
        // compare types
        auto lhsType = ex->lhs->resultType.get();
        auto rhsType = ex->rhs->resultType.get();
        if (lhsType->equals(rhsType)) {
          ex->resultType = lhsType->clone();
          return true;
        }
        else {
          error("types of binary expression do not match: lhs type '"
            + lhsType->toString() + "' and rhs type '"
            + rhsType->toString() + "'",
            ex->location);
          return false;
        }
      }

      error("unsupported expression ", expression->location);
      return false;
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

      ex->resultType = varDecl->type->clone();
      return true;
    }

    bool doCallExpression(CallExpression *call, bool variableExpressionsAllowed) {
      auto func = dynamic_cast<FunctionDeclaration*>(namesStack.findName(call->calledName));
      if (!func) {
        error("function with name '"+ call->calledName +"' not declared", call->location);
        return false;
      }

      // params
      for (auto &arg : call->arguments) {
        // @todo
      }

      // return type
      if (!func->returnType) {
        return false;
      }
      else {
        call->resultType = func->returnType->clone();
      }

      return true;
    }


    void doStatement(Statement *statement, NamesScope &scope, Type *expectedTypeForReturn) {
      // return
      if (auto* st = dynamic_cast<ReturnStatement*>(statement)) {
        bool exprOk = doExpression(st->expression->get(), true);
        if (exprOk && !st->expression->get()->resultType->equals(expectedTypeForReturn)) {
          string exprType = st->expression ? st->expression->get()->resultType->toString() : "void";
          error("expected return type '"+ expectedTypeForReturn->toString() +"' for function "
               +"does not match given return type '"+ exprType +"'", statement->location);
        }
      }
      // variable declaration
      else if (auto* st = dynamic_cast<VariableDeclaration*>(statement)) {
        doVariableDeclaration(st, true);
        if (!scope.addName(st->name, *st)) {
          error("name '" + st->name + "' already declared", st->location)
              .printMessage("name '" + st->name + "' previously declared here", scope.findName(st->name)->location);
        }
      }
      else if (auto* st = dynamic_cast<Expression*>(statement)) {
        doExpression(st, true);
      }
      else {
        error("unsupported statement", statement->location);
      }
    }

    void doVariableDeclaration(VariableDeclaration *varDecl, bool variableExpressionsAllowedInInit) {
      // resolve initExpression
      bool initOk = doExpression(varDecl->initExpression.get(), variableExpressionsAllowedInInit);
      Type *initExprType = varDecl->initExpression->resultType.get();
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
    unique_ptr<Type> makeTypeForName(string &name, SrcLocationRange &location) {
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