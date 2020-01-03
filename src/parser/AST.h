#pragma once

#include<iostream>
using namespace std;

class VariableDeclaration;
class FunctionDeclaration;
class FunctionParamDeclaration;

class ASTNode {
  public:
    SrcLocationRange location = SrcLocationRange(SrcLocation(-1,-1,-1));

    virtual void print(int depth) {
      cout << depthToTabs(depth) << "ASTNode() at " << location.toString() << endl;
    }

    static string depthToTabs(int depthLevel) {
      string s;
      for (int i = 0; i < depthLevel; ++i)
      {
        s+= "    ";
      }
      return s;
    }
};




/**
 * Types
 */
class Type {
  public:
    virtual std::unique_ptr<Type> clone() const = 0;

    virtual void print(int depth) {
      cout << ASTNode::depthToTabs(depth) << "Type(" << toString() << ")" << endl;
    }

    virtual string toString() = 0;

    virtual bool equals(Type *other) = 0;

    virtual ~Type() = default;
};

class UserDefinedType: public Type {
  public:
    string name;
    ASTNode *declaration;
};


enum BUILD_IN_TYPE {
    BuildIn_No_BuildIn = -1,
    BuildIn_i32,
    BuildIn_f32,
    BuildIn_void,
    BuildIn_bool,
};
class BuildInType: public Type {
  public:
    BUILD_IN_TYPE type = BuildIn_No_BuildIn;

    BuildInType(BUILD_IN_TYPE type) : type(type)
    {}

    bool equals(Type *other) override{
      if (auto* o = dynamic_cast<BuildInType*>(other)) {
        return o->type == this->type;
      }
      return false;
    }

    virtual std::unique_ptr<Type> clone() const override{
      return make_unique<BuildInType>(*this);
    }

    string toString() override
    {
      return "" + string(magic_enum::enum_name(type));
    }
};

class TypeNode: public ASTNode {
  public:
    string typeName;
    unique_ptr<Type> type;
};




class Statement: public ASTNode {

};


class Expression: public Statement {
  public:
    unique_ptr<Type> resultType;

    void printType(int depth) {
      if (resultType) {
        resultType->print(depth + 1);
      }
    }
};


/**
 * Op value is its precedence.
 * 1 is the lowest precedence.
 */
enum BinaryExpressionOp {
    Expr_Op_Invalid = -1,
    EXPR_OP_GREATER_THEN = 10,
    EXPR_OP_SMALLER_THEN = 11,
    Expr_Op_Plus = 20,
    Expr_Op_Minus = 30,
    Expr_Op_Divide = 40,
    Expr_Op_Multiply = 50,
};

class BinaryExpression: public Expression {
  public:


    static BinaryExpressionOp tokenToBinaryExpressionOp(TOKEN_TYPE type) {
      switch (type) {
        case Operator_Plus:
          return Expr_Op_Plus;
        case Operator_Minus:
          return Expr_Op_Minus;
        case Operator_Multiply:
          return Expr_Op_Multiply;
        case Operator_Divide:
          return Expr_Op_Divide;

        default:
          return Expr_Op_Invalid;
      }
    }

    unique_ptr<Expression> lhs;
    unique_ptr<Expression> rhs;
    BinaryExpressionOp operation;

    void print(int depth) override {
      cout << depthToTabs(depth) << "BinaryExpression(operation: " << magic_enum::enum_name(operation) << ") at " << location.toString() << endl;
      printType(depth);
      cout << depthToTabs(depth) << "> lhs:" << endl;
      lhs->print(depth + 1);
      cout << depthToTabs(depth) << "> rhs:" << endl;
      rhs->print(depth + 1);
    }
};

class VariableExpression: public Expression {
  public:
    string name;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableExpression(name: " << name << ") at " << location.toString() << endl;
      printType(depth);
    }
};

class NumberExpression: public Expression {
};

class NumberIntExpression: public NumberExpression {
  public:
    int value = 0;

    void print(int depth) override {
      cout << depthToTabs(depth) << "NumberIntExpression(value: " << value << ") at " << location.toString() << endl;
      printType(depth);
    }
};

class NumberFloatExpression: public NumberExpression {
  public:
    float value = 0;

    void print(int depth) override {
      cout << depthToTabs(depth) << "NumberFloatExpression(value: " << value << ") at " << location.toString() << endl;
      printType(depth);
    }
};


class StringExpression: public Expression {
  public:
    string value;

    void print(int depth) override {
      cout << depthToTabs(depth) << "StringExpression(value: " << value << ") at " << location.toString() << endl;
      printType(depth);
    }
};

class CallExpressionArgument: public ASTNode {
  public:
    unique_ptr<Expression> expression;
    optional<string> argName = nullopt;
    FunctionParamDeclaration *argumentDeclaration;

    void print(int depth) override {
      if (argName) {
        cout << depthToTabs(depth) << "CallExpressionArgument(argName: " << *argName << ") at " << location.toString() << endl;
      } else {
        cout << depthToTabs(depth) << "CallExpressionArgument(UNNAMED) at " << location.toString() << endl;
      }
      cout << depthToTabs(depth) << "> expression:" << endl;
      expression->print(depth + 1);
    }
};

class CallExpression: public Expression {
  public:
    string calledName;
    vector<CallExpressionArgument> argumentsNonNamed;
    vector<CallExpressionArgument> argumentsNamed;
    FunctionDeclaration *functionDeclaration;

    void print(int depth) override {
      cout << depthToTabs(depth) << "CallExpression(calledName: " << calledName << ") at " << location.toString() << endl;
      if (!argumentsNonNamed.empty()) {
        cout << depthToTabs(depth) << "> arguments:" << endl;
        for (auto &a : argumentsNonNamed)
          a.print(depth + 1);
      }
      if (!argumentsNamed.empty()) {
        cout << depthToTabs(depth) << "> arguments-named:" << endl;
        for (auto &a : argumentsNamed)
          a.print(depth + 1);
      }
    }
};




class AbstractVariableDeclaration {
  public:
    string name;
    string typeName;
    unique_ptr<Type> type;
};

class VariableDeclaration: public Statement, public AbstractVariableDeclaration {
  public:
    unique_ptr<Expression> initExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableDeclaration(name: " << name << ", type: " << typeName << ") at " << location.toString() << endl;
      if (type) {
        cout << depthToTabs(depth) << "> type:" << endl;
        type->print(depth + 1);
      }
      cout << depthToTabs(depth) << "> init:" << endl;
      initExpression->print(depth + 1);
    }
};

class ReturnStatement: public Statement {
  public:
    optional<unique_ptr<Expression>> expression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "ReturnStatement() at " << location.toString() << endl;
      if (expression) {
        cout << depthToTabs(depth) << "> expression:" << endl;
        expression->get()->print(depth + 1);
      }
    }
};


class FunctionParamDeclaration: public ASTNode, public AbstractVariableDeclaration {
  public:
    shared_ptr<Expression> defaultExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "FunctionParamDeclaration(name: " << name << ", type: " << typeName << ") at " << location.toString() << endl;
      if (type) {
        cout << depthToTabs(depth) << "> type:" << endl;
        type->print(depth + 1);
      }
      if (defaultExpression) {
        cout << depthToTabs(depth) << "> default:" << endl;
        defaultExpression->print(depth + 1);
      }
    }
};

class FunctionDeclaration: public ASTNode {
  public:
    string name;
    string typeName;
    unique_ptr<Type> returnType;
    vector<FunctionParamDeclaration> arguments;
    vector<unique_ptr<Statement>> bodyStatements;

    void print(int depth) override {
      cout << depthToTabs(depth) << "FunctionDeclaration(name: " << name << ", type: " << typeName << ") at " << location.toString() << endl;
      if (returnType) {
        cout << depthToTabs(depth) << "> type:" << endl;
        returnType->print(depth + 1);
      }
      if (!arguments.empty()) {
        cout << depthToTabs(depth) << "> arguments:" << endl;
        for (auto &a : arguments)
          a.print(depth + 1);
      }
      if (!bodyStatements.empty()) {
        cout << depthToTabs(depth) << "> body:" << endl;
        for (auto &s : bodyStatements)
          s->print(depth + 1);
      }
    }
};



class RootDeclarations: public ASTNode {
  public:
    list<unique_ptr<VariableDeclaration>> variableDeclarations;
    list<FunctionDeclaration> functionDeclarations;
    FunctionDeclaration *mainFunction = nullptr;

    void print(int depth) override {
      cout << depthToTabs(depth) << "RootDeclarations() at " << location.toString() << endl;
      cout << depthToTabs(depth) << "> global vars:" << endl;
      for (auto &var : variableDeclarations) {
        var->print(depth + 1);
      }

      cout << depthToTabs(depth) << "> functions:" << endl;
      for (auto &var : functionDeclarations) {
        var.print(depth + 1);
      }
    }

    void print() {
      print(0);
    }
};



