#pragma once

#include<iostream>
#include "llvm/IR/Function.h"
#include "util/util.h"
#include "Types.h"
#include "lexer/Lexer.h"

using namespace std;

class VariableDeclaration;
class FunctionDeclaration;
class FunctionParamDeclaration;
class AbstractVariableDeclaration;
class ClassDeclaration;


/**
 * Base for all AstNodes.
 */
class ASTNode {
  public:
    SrcLocationRange location = SrcLocationRange(SrcLocation(-1,-1,-1));

    virtual ~ASTNode() {}


    /**
     * Get the type name of this node.
     */
    virtual string nodeName() = 0;



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


class ReferenceType: public LangType {
  public:
    unique_ptr<LangType> innerType;

    ReferenceType(unique_ptr<LangType> innerType) : innerType(move(innerType))
    {}

    ReferenceType(const ReferenceType &original) : innerType(original.innerType->clone())
    { }

    std::unique_ptr<LangType> clone() const override {
      return make_unique<ReferenceType>(*this);
    }

    string toString() override {
      return "Reference<"+ innerType->toString() +">";
    }
    bool equals(LangType *other) override {
      if (auto* o = dynamic_cast<ReferenceType*>(other)) {
        return o->innerType->equals(this->innerType.get());
      }
      return false;
    }
};


class UserDefinedType: public LangType {
};

class ClassType: public UserDefinedType {
  public:
    ClassDeclaration *classDeclaration;

    explicit ClassType(ClassDeclaration *classDecl): classDeclaration(classDecl)
    { }

    std::unique_ptr<LangType> clone() const override {
      return make_unique<ClassType>(*this);
    }

    string toString() override;

    bool equals(LangType *other) override {
      if (auto* o = dynamic_cast<ClassType*>(other)) {
        return o->classDeclaration == this->classDeclaration;
      }
      return false;
    }

    bool isClassType() override {
      return true;
    }
};



enum BUILD_IN_TYPE {
    BuildIn_No_BuildIn = -1,
    BuildIn_i32,
    BuildIn_f32,
    BuildIn_void,
    BuildIn_bool,
    // static string (has contend and length)
    BuildIn_str, // @todo will be replaced by predefined str class
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
};




class Statement: public ASTNode {

};



/**************************************************************
 *** Expressions
 */

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
 * Ops of unary expression
 */
enum UnaryExpressionOp {
    Expr_Unary_Op_Invalid = -1,
    Expr_Unary_Op_LOGIC_NOT,
};

string toString(UnaryExpressionOp op) {
  switch (op) {
    case Expr_Unary_Op_Invalid:
      return "UNARY_OP_INVALID";
    case Expr_Unary_Op_LOGIC_NOT:
      return "!";
  }
}

class UnaryExpression: public Expression {
  public:
    static UnaryExpressionOp tokenToBinaryExpressionOp(TOKEN_TYPE type) {
      switch (type) {
        case Operator_Unary_Not:
          return Expr_Unary_Op_Invalid;
        default:
          return Expr_Unary_Op_LOGIC_NOT;
      }
    }

    unique_ptr<Expression> innerExpression;
    UnaryExpressionOp operation;

    string nodeName() override {
      return "UnaryExpression";
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

string toString(BinaryExpressionOp op) {
  switch (op) {
    case Expr_Op_Invalid:
      return "BINARY_OP_INVALID";
    case EXPR_OP_LOGIC_OR:
      return "||";
    case EXPR_OP_LOGIC_AND:
      return "&&";
    case EXPR_OP_EQUALS:
      return "==";
    case EXPR_OP_NOT_EQUALS:
      return "!=";
    case EXPR_OP_GREATER_THEN:
      return ">";
    case EXPR_OP_GREATER_EQUALS_THEN:
      return ">=";
    case EXPR_OP_LESS_THEN:
      return "<";
    case EXPR_OP_LESS_EQUALS_THEN:
      return "<=";
    case Expr_Op_Plus:
      return "+";
    case Expr_Op_Minus:
      return "-";
    case Expr_Op_Divide:
      return "/";
    case Expr_Op_Multiply:
      return "*";
  }
}

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

class IdentifierExpression: public Expression {

};

class VariableExpression: public IdentifierExpression {
  public:
    string name;
    /** links to declaration of the variable */
    AbstractVariableDeclaration *variableDeclaration;

    string nodeName() override {
      return "VariableExpression";
    }
};

/**
 * Expression like myObject.prop
 * '.prop' is a MemberExpression
 * and 'myObject' is parent of '.prop' and a VariableExpression
 */
class MemberVariableExpression: public VariableExpression {
  public:
    /** parent of the member */
    unique_ptr<IdentifierExpression> parent;

    string nodeName() override {
      return "MemberVariableExpression";
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

    string nodeName() override {
      return "NumberIntExpression";
    }
};

class NumberFloatExpression: public NumberExpression {
  public:
    float value = 0;

    string nodeName() override {
      return "NumberFloatExpression";
    }
};

class StringExpression: public ConstValueExpression {
  public:
    string value;

    string nodeName() override {
      return "StringExpression";
    }
};


class BoolExpression: public ConstValueExpression {
  public:
    bool value;

    BoolExpression() = default;
    BoolExpression(bool value) : value(value)
    {}

    string nodeName() override {
      return "BoolExpression";
    }
};



class CallExpressionArgument: public ASTNode {
  public:
    /** the value of the function argument at the specific call */
    shared_ptr<Expression> expression;

    /** contains the argument name if arg was used with that name like 'arg1=value' */
    optional<string> argName = nullopt;

    /** links to declaration of the function argument */
    FunctionParamDeclaration *argumentDeclaration;

    string nodeName() override {
      return "CallExpressionArgument";
    }
};

class CallExpression: public IdentifierExpression {
  public:
    string calledName;
    vector<CallExpressionArgument> argumentsNonNamed;
    vector<CallExpressionArgument> argumentsNamed;

    /** links to the function that is called */
    FunctionDeclaration *functionDeclaration;

    string nodeName() override {
      return "CallExpression";
    }
};


/**
 * Expression like myObject.func()
 * '.func()' is a MemberExpression
 * and 'myObject' is parent of '.func()' and a VariableExpression
 */
class MemberCallExpression: public CallExpression {
  public:
    /** parent of the member */
    unique_ptr<IdentifierExpression> parent;

    string nodeName() override {
      return "MemberCallExpression";
    }
};




/**************************************************************
 *** Statements
 */

class AbstractVariableDeclaration {
  public:
    string name;
    string typeName;
    unique_ptr<LangType> type;
    llvm::Value *llvmVariable;
    bool isMutable = true;

    /** links to the parent class if it is a member function */
    ClassDeclaration *parentClass = nullptr;
    /** if its a class member */
    int memberIndex = -1;

    /** if its this argument of a class, then parentClass points to the class decl */
    bool isThisOfClass = false;

    bool isMemberVariable() {
      return parentClass != nullptr;
    }
};

class VariableDeclaration: public Statement, public AbstractVariableDeclaration {
  public:
    /** optional, can be none when its a class member */
    unique_ptr<Expression> initExpression;

    string nodeName() override {
      return "VariableDeclaration";
    }
};

class ReturnStatement: public Statement {
  public:
    /** the return value */
    optional<unique_ptr<Expression>> expression;
    unique_ptr<LangType> returnType;

    string nodeName() override {
      return "ReturnStatement";
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

    string nodeName() override {
      return "CompoundStatement";
    }
};


class IfStatement: public Statement {
  public:
    unique_ptr<Expression> condition;
    unique_ptr<CompoundStatement> ifBody;
    unique_ptr<CompoundStatement> elseBody;

    string nodeName() override {
      return "IfStatement";
    }
};


class WhileStatement: public Statement {
  public:
    unique_ptr<Expression> condition;
    unique_ptr<CompoundStatement> body;

    string nodeName() override {
      return "WhileStatement";
    }
};


class VariableAssignStatement: public Statement {
  public:
    unique_ptr<Expression> valueExpression;
    unique_ptr<VariableExpression> variableExpression;

    string nodeName() override {
      return "VariableAssignStatement";
    }
};





class FunctionParamDeclaration: public ASTNode, public AbstractVariableDeclaration {
  public:
    shared_ptr<Expression> defaultExpression;

    string nodeName() override {
      return "FunctionParamDeclaration";
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

    /** links to the parent class if it is a member function */
    ClassDeclaration *parentClass = nullptr;
    bool isConstructor = false;

    bool isMemberFunction() {
      return parentClass != nullptr;
    }

    string nodeName() override {
      return "FunctionDeclaration";
    }
};



class ClassDeclaration: public ASTNode {
  public:
    string name;
    list<unique_ptr<VariableDeclaration>> variableDeclarations;
    unique_ptr<VariableDeclaration> thisVarDecl;
    list<FunctionDeclaration> functionDeclarations;
    FunctionDeclaration constructor;

    /** llvm type for this class */
    llvm::StructType *llvmStructType = nullptr;
    int llvmStructSizeBytes = -1;


    string nodeName() override {
      return "ClassDeclaration";
    }

    /**
     * Find a member variable by name.
     */
    VariableDeclaration *findMemberVariable(string &name) {
      auto found = find_if(variableDeclarations.begin(), variableDeclarations.end(),
          [&](auto &var) {
            return var->name == name;
      });
      if (found == variableDeclarations.end()) {
        return nullptr;
      }
      return found->get();
    }

    /**
     * Find a member function by name.
     */
    FunctionDeclaration *findMemberFunction(string &name) {
      auto found = find_if(functionDeclarations.begin(), functionDeclarations.end(),
                           [&](const FunctionDeclaration &var) {
                             return var.name == name;
                           });
      if (found == functionDeclarations.end()) {
        return nullptr;
      }
      return &*found;
    }
};



class RootDeclarations: public ASTNode {
  public:
    list<unique_ptr<VariableDeclaration>> variableDeclarations;
    list<FunctionDeclaration> functionDeclarations;
    list<unique_ptr<ClassDeclaration>> classDeclarations;
    FunctionDeclaration *mainFunction = nullptr;

    string nodeName() override {
      return "RootDeclarations";
    }
};




// LangTypes
string ClassType::toString()
{
  return "class_" + classDeclaration->name;
}