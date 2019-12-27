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


class Expression: public ASTNode {
  public:

};

class BinaryExpression: public Expression {
  public:
    /**
     * Op value is its precedence.
     * 1 is the lowest precedence.
     */
    enum BinaryExpressionOp {
        EXPR_OP_GREATER_THEN = 10,
        EXPR_OP_SMALLER_THEN = 11,
        EXPR_OP_PLUS = 20,
        EXPR_OP_MINUS = 30,
        EXPR_OP_DIVIDE = 40,
        EXPR_OP_MUL = 50,
    };

    unique_ptr<Expression> lhs;
    unique_ptr<Expression> rhs;
    BinaryExpressionOp operation;

    void print(int depth) override {
      cout << depthToTabs(depth) << "BinaryExpression(operation: " << magic_enum::enum_name(operation) << ")" << endl;
      cout << depthToTabs(depth) << "> left hand side:" << endl;
      lhs->print(depth + 1);
      cout << depthToTabs(depth) << "> right hand side:" << endl;
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

class Statement: public ASTNode {

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



