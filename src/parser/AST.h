#pragma once

#include<iostream>
#include "llvm/IR/Function.h"
#include "util/util.h"

using namespace std;

class VariableDeclaration;
class FunctionDeclaration;
class FunctionParamDeclaration;
class AbstractVariableDeclaration;

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
class LangType {
  public:
    virtual std::unique_ptr<LangType> clone() const = 0;

    virtual void print(int depth) {
      cout << ASTNode::depthToTabs(depth) << "Type(" << toString() << ")" << endl;
    }

    virtual string toString() = 0;

    virtual bool equals(LangType *other) = 0;

    virtual bool isInvalid()
    { return false; }
    virtual bool isVoidType()
    { return false; }
    virtual bool isNumericalType()
    { return false; }

    virtual ~LangType() = default;
};

class InvalidType: public LangType {
  public:
    unique_ptr<LangType> clone() const override{
      return std::unique_ptr<InvalidType>();
    }
    string toString() override{
      return "InvalidType";
    }
    bool equals(LangType *other) override {
      return false;
    }
    bool isInvalid() override
    { return true; }
};

class UserDefinedType: public LangType {
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
class BuildInType: public LangType {
  public:
    BUILD_IN_TYPE type = BuildIn_No_BuildIn;

    BuildInType(BUILD_IN_TYPE type) : type(type)
    {}

    bool equals(LangType *other) override{
      if (auto* o = dynamic_cast<BuildInType*>(other)) {
        return o->type == this->type;
      }
      return false;
    }

    virtual std::unique_ptr<LangType> clone() const override{
      return make_unique<BuildInType>(*this);
    }

    string toString() override
    {
      string t = string(magic_enum::enum_name(type));
      stringRemovePrefix(t, "BuildIn_");
      return t;
    }

    bool isVoidType() override {
      return type == BuildIn_void;
    }

    bool isNumericalType() override {
      switch (type) {
        case BuildIn_i32:
        case BuildIn_f32:
          return true;
        default:
          return false;
      }
    }
};

class TypeNode: public ASTNode {
  public:
    string typeName;
    unique_ptr<LangType> type;
};




class Statement: public ASTNode {

};


class Expression: public Statement {
  public:
    unique_ptr<LangType> resultType;

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
    EXPR_OP_LOGIC_OR = 5,
    EXPR_OP_LOGIC_AND = 10,
    EXPR_OP_EQUALS = 20,
    EXPR_OP_NOT_EQUALS = 25,
    EXPR_OP_GREATER_THEN = 30,
    EXPR_OP_GREATER_EQUALS_THEN = 35,
    EXPR_OP_LESS_THEN = 40,
    EXPR_OP_LESS_EQUALS_THEN = 45,
    Expr_Op_Plus = 50,
    Expr_Op_Minus = 60,
    Expr_Op_Divide = 70,
    Expr_Op_Multiply = 80,
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
        case Operator_Equals:
          return EXPR_OP_EQUALS;
        case Operator_NotEquals:
          return EXPR_OP_NOT_EQUALS;
        case Operator_GreaterThen:
          return EXPR_OP_GREATER_THEN;
        case Operator_GreaterEqualThen:
          return EXPR_OP_GREATER_EQUALS_THEN;
        case Operator_LessThen:
          return EXPR_OP_LESS_THEN;
        case Operator_LessEqualThen:
          return EXPR_OP_LESS_EQUALS_THEN;
        case Operator_LogicOr:
          return EXPR_OP_LOGIC_OR;
        case Operator_LogicAnd:
          return EXPR_OP_LOGIC_AND;

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
    AbstractVariableDeclaration *variableDeclaration;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableExpression(name: " << name << ") at " << location.toString() << endl;
      printType(depth);
    }
};

class ConstValueExpression: public Expression {
  public:
    ConstValueExpression()= default;
    ConstValueExpression(const ConstValueExpression &expression){
      this->location = expression.location;
      this->resultType = expression.resultType->clone();
    }

    /*
    virtual std::unique_ptr<ConstValueExpression> clone() const {
      return make_unique<ConstValueExpression>(*this);
    }
     */
};

class NumberExpression: public ConstValueExpression {
};

class NumberIntExpression: public NumberExpression {
  public:
    int32_t value = 0;

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

class StringExpression: public ConstValueExpression {
  public:
    string value;

    void print(int depth) override {
      cout << depthToTabs(depth) << "StringExpression(value: " << value << ") at " << location.toString() << endl;
      printType(depth);
    }
};


class BoolExpression: public ConstValueExpression {
  public:
    bool value;

    BoolExpression() = default;
    BoolExpression(bool value) : value(value)
    {}

    void print(int depth) override {
      cout << depthToTabs(depth) << "BoolExpression(value: " << value << ") at " << location.toString() << endl;
      printType(depth);
    }
};



class CallExpressionArgument: public ASTNode {
  public:
    shared_ptr<Expression> expression;
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
      printType(depth);
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
    unique_ptr<LangType> type;
    llvm::Value *llvmVariable;

    bool isMutable = true;
};

class VariableDeclaration: public Statement, public AbstractVariableDeclaration {
  public:
    unique_ptr<Expression> initExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableDeclaration(name: " << name << ", type: " << typeName << ", mutable: "<< (isMutable ? "TRUE" : "FALSE") <<") at " << location.toString() << endl;
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
    unique_ptr<LangType> returnType;

    void print(int depth) override {
      cout << depthToTabs(depth) << "ReturnStatement() at " << location.toString() << endl;
      if (expression) {
        cout << depthToTabs(depth) << "> expression:" << endl;
        expression->get()->print(depth + 1);
      }
    }
};


/**
 * Contains multiple statements in a scope:
 * {
 *   let a = 1;
 *   let b = a;
 *   ...
 * }
 */
class CompoundStatement: public Statement {
  public:
    vector<unique_ptr<Statement>> statements;

    void print(int depth) override {
      cout << depthToTabs(depth) << "CompoundStatement() at " << location.toString() << endl;
      if (!statements.empty()) {
        for (auto &s : statements)
          s->print(depth + 1);
      }
    }
};


class IfStatement: public Statement {
  public:
    unique_ptr<Expression> condition;
    unique_ptr<CompoundStatement> ifBody;
    unique_ptr<CompoundStatement> elseBody;

    void print(int depth) override {
      cout << depthToTabs(depth) << "IfStatement() at " << location.toString() << endl;
      cout << depthToTabs(depth) << "> condition:" << endl;
      condition->print(depth + 1);
      cout << depthToTabs(depth) << "> if body:" << endl;
      ifBody->print(depth + 1);
      if (elseBody) {
        cout << depthToTabs(depth) << "> else body:" << endl;
        elseBody->print(depth + 1);
      }
    }
};


class WhileStatement: public Statement {
  public:
    unique_ptr<Expression> condition;
    unique_ptr<CompoundStatement> body;

    void print(int depth) override {
      cout << depthToTabs(depth) << "WhileStatement() at " << location.toString() << endl;
      cout << depthToTabs(depth) << "> condition:" << endl;
      condition->print(depth + 1);
      cout << depthToTabs(depth) << "> body:" << endl;
      body->print(depth + 1);
    }
};


class VariableAssignStatement: public Statement {
  public:
    unique_ptr<Expression> valueExpression;
    unique_ptr<VariableExpression> variableExpression;

    void print(int depth) override {
      cout << depthToTabs(depth) << "VariableAssignStatement() at " << location.toString() << endl;
      cout << depthToTabs(depth) << "> variable:" << endl;
      variableExpression->print(depth + 1);
      cout << depthToTabs(depth) << "> value:" << endl;
      valueExpression->print(depth + 1);
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
    bool isExtern;
    unique_ptr<LangType> returnType;
    vector<FunctionParamDeclaration> arguments;
    unique_ptr<CompoundStatement> body;
    llvm::Function *llvmFunction;

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
      if (body) {
        cout << depthToTabs(depth) << "> body:" << endl;
        body->print(depth +1);
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



