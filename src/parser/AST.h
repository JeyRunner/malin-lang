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
    enum BinaryExpressionOp {
        EXPR_OP_MUL = 10,
        EXPR_OP_DIVIDE = 20,
        EXPR_OP_PLUS = 30,
        EXPR_OP_MINUS = 40,
    };

    unique_ptr<Expression> lhs;
    unique_ptr<Expression> rhs;
    BinaryExpressionOp operation;
};

class VariableExpression: public Expression {
  public:
    string name;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableExpression(name: " << name << ")" << endl;
    }
};

class NumberExpression: public Expression {
  public:
    double value = 0;

    void print(int depth) override {
      cout << depthToTabs(depth) << "NumberExpression(value: " << value << ")" << endl;
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

class VariableDeclaration: Statement {
  public:
    string name;
    string typeName;
    unique_ptr<Expression> initExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableDeclaration(name: " << name << ", type: " << typeName << ")" << endl;
      cout << depthToTabs(depth) << "+- init:" << endl;
      initExpression->print(depth + 1);
    }
};

class RootDeclarations: public ASTNode {
  public:
    list<VariableDeclaration> variableDeclarations;

    void print(int depth) override {
      cout << depthToTabs(depth) << "RootDeclarations()" << endl;
      cout << depthToTabs(depth) << "+- vars:" << endl;
      for (auto &var : variableDeclarations) {
        var.print(depth + 1);
      }
    }

    void print() {
      print(0);
    }
};



