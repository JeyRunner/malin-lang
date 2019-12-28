#pragma once

#include<iostream>
using namespace std;



class ASTNode {
  public:
    virtual void print(int depth) {
      cout << depthToTabs(depth) << "ASTNode()" << endl;
    }

  protected:
    static string depthToTabs(int depthLevel) {
      string s;
      for (int i = 0; i < depthLevel; ++i)
      {
        s+= "    ";
      }
      return s;
    }
};


class Statement: public ASTNode {

};


class Expression: public Statement {
  public:

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
      cout << depthToTabs(depth) << "BinaryExpression(operation: " << magic_enum::enum_name(operation) << ")" << endl;
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
      cout << depthToTabs(depth) << "VariableExpression(name: " << name << ")" << endl;
    }
};

class NumberExpression: public Expression {
};

class NumberIntExpression: public NumberExpression {
  public:
    int value = 0;

    void print(int depth) override {
      cout << depthToTabs(depth) << "NumberIntExpression(value: " << value << ")" << endl;
    }
};

class NumberFloatExpression: public NumberExpression {
  public:
    float value = 0;

    void print(int depth) override {
      cout << depthToTabs(depth) << "NumberFloatExpression(value: " << value << ")" << endl;
    }
};


class StringExpression: public Expression {
  public:
    string value;

    void print(int depth) override {
      cout << depthToTabs(depth) << "StringExpression(value: " << value << ")" << endl;
    }
};

class CallExpressionArgument: public ASTNode {
  public:
    unique_ptr<Expression> expression;
    optional<string> argName = nullopt;

    void print(int depth) override {
      if (argName) {
        cout << depthToTabs(depth) << "CallExpressionArgument(argName: " << *argName << ")" << endl;
      } else {
        cout << depthToTabs(depth) << "CallExpressionArgument(UNNAMED)" << endl;
      }
      cout << depthToTabs(depth) << "> expression:" << endl;
      expression->print(depth + 1);
    }
};

class CallExpression: public Expression {
  public:
    string calledName;
    vector<CallExpressionArgument> arguments;

    void print(int depth) override {
      cout << depthToTabs(depth) << "CallExpression(calledName: " << calledName << ")" << endl;
      if (!arguments.empty()) {
        cout << depthToTabs(depth) << "> arguments:" << endl;
        for (auto &a : arguments)
          a.print(depth + 1);
      }
    }
};






class VariableDeclaration: public Statement {
  public:
    string name;
    string typeName;
    unique_ptr<Expression> initExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableDeclaration(name: " << name << ", type: " << typeName << ")" << endl;
      cout << depthToTabs(depth) << "> init:" << endl;
      initExpression->print(depth + 1);
    }
};


class FunctionParamDeclaration: public ASTNode {
  public:
    string name;
    string typeName;
    unique_ptr<Expression> defaultExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "FunctionParamDeclaration(name: " << name << ", type: " << typeName << ")" << endl;
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
    vector<FunctionParamDeclaration> arguments;
    vector<unique_ptr<Statement>> bodyStatements;

    void print(int depth) override {
      cout << depthToTabs(depth) << "FunctionDeclaration(name: " << name << ", type: " << typeName << ")" << endl;
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

    void print(int depth) override {
      cout << depthToTabs(depth) << "RootDeclarations()" << endl;
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



