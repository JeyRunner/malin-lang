#pragma once

#include <iostream>
#include "llvm/IR/Function.h"
#include "ir/IRValueVar.h"
#include "util/util.h"
#include "Types.h"
#include "lexer/Lexer.h"
#include "AstIterator/AstNodeChildIterator.h"
#include "AstReplacable.h"

using namespace std;

class VariableDeclaration;
class FunctionDeclaration;
class FunctionParamDeclaration;
class AbstractVariableDeclaration;
class ClassDeclaration;
class Expression;
class VariableExpression;


/**
 * Base for all AstNodes.
 */
class ASTNode {
  public:
    SrcLocationRange location = SrcLocationRange(SrcLocation(-1,-1,-1));

    /** the parent node of this node in the ast tree.
     * For the RootDeclarations node and direct children of RootDeclarations this is null
     * NOTE: AFTER MOVING THE NODE THIS HAS TO BE RE-ASSIGNED!
     */
    ASTNode *parentAstNode = nullptr;

    virtual ~ASTNode() {}


    /**
     * Get the type name of this node.
     */
    virtual string nodeName() = 0;

    /**
     * Get all ast node children of this node.
     */
    virtual AstChildRange getChildNodes() = 0;

    /**
     * Replace a direct child of the node
     * @return
     *-/
    template <typename OLD, typename NEW,  typename std::enable_if<std::is_base_of<OLD, NEW>::value>::type* = nullptr>
    bool replaceChildT(OLD *toReplace, unique_ptr<NEW> replaceWith) {
      return replaceChild(toReplace, move(replaceWith));
    }
     */


    /*
     * Replace a direct expression child of the node.
     * If the node has no expression children a runtime_error is thrown.
     * @return true if the child expression node was found, false otherwise.
     *-/
    virtual bool replaceChildExpressionNode(Expression *toReplace, unique_ptr<Expression> &&replaceWith) {
      throw runtime_error(string("method 'replaceChildExpressionNode(toReplace, replaceWith)' not implemented for ASTNode '") + nodeName()
        + "', but was called.");
      return false;
    }
     */
};




class Statement: public ASTNode {

};



/**************************************************************
 *** Expressions
 */

/// @note replaceNode currently only works with type extending VariableExpression because self3 is VariableExpression
/// to fix this Child classes have to extend Replacable not Expression
class Expression: public Statement, public Replacable<Expression, Expression, Statement, VariableExpression> {
  public:
    unique_ptr<LangType> resultType;

    /**
     * points to the unique pointer that owns this Expression object, use this to replace the expression.
     * \note AFTER MOVING OR COPYING THE NODE THIS HAS TO BE RE-ASSIGNED!
     */
    //unique_ptr<Expression> *self = nullptr;
    //unique_ptr<Statement> *selfInStatement = nullptr;

    void printType(int depth) {
      if (resultType) {
        resultType->print(depth + 1);
      }
    }

    /*
    void replace(unique_ptr<Expression> replaceWith) {
      if (self)
        *self = move(replaceWith);
      else if (selfInStatement)
        *selfInStatement = move(replaceWith);
    }
     */
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

    AstChildRange getChildNodes() override {
      return makeAstRange({innerExpression.get()});
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


    string nodeName() override {
      return "BinaryExpression";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({lhs.get(), rhs.get()});
    }


    /*
     * @deprecated
     * Replace a direct expression child of the node.
     * If the node has no expression children a runtime_error is thrown.
     * @return true if the child expression node was found, false otherwise.
     *-/
    bool replaceChildExpressionNode(Expression *toReplace, unique_ptr<Expression> &&replaceWith) override {
      if (lhs.get() == toReplace) {
        lhs = dynamic_unique_pointer_cast_throwing<Expression>(move(replaceWith));
        return true;
      }
      if (rhs.get() == toReplace) {
        rhs = dynamic_unique_pointer_cast_throwing<Expression>(move(replaceWith));
        return false;
      }
      return false;
    }
    */
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

    AstChildRange getChildNodes() override {
      return makeAstRange({});
    }
};

/**
 * Expression like myObject.prop
 * '.prop' is a MemberExpression
 * and 'myObject' is parent of '.prop' and a VariableExpression
 */
class MemberVariableExpression: public VariableExpression {
  public:
    /** parent of the member has type  IdentifierExpression */
    // @todo make type IdentifierExpression
    unique_ptr<Expression> parent; // IdentifierExpression

    string nodeName() override {
      return "MemberVariableExpression";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({parent.get()});
    }
};


class ConstValueExpression: public Expression {
  public:
    ConstValueExpression()= default;

    ConstValueExpression(const ConstValueExpression &expression){
      this->location = expression.location;
      this->resultType = expression.resultType->clone();
    }


    /**
     * Copy this const expression.
     * NOTE: AFTERWARDS 'parentAstNode' AND 'self' HAVE TO BE RE-ASSIGNED!
     * @return
     */
    virtual std::unique_ptr<ConstValueExpression> clone() const = 0;

    AstChildRange getChildNodes() override
    { return makeAstRange({}); }
};

class NumberExpression: public ConstValueExpression {
};

class NumberIntExpression: public NumberExpression {
  public:
    int32_t value = 0;

    string nodeName() override {
      return "NumberIntExpression";
    }

    std::unique_ptr<ConstValueExpression> clone() const override {
      return make_unique<NumberIntExpression>(*this);
    }
};

class NumberFloatExpression: public NumberExpression {
  public:
    float value = 0;

    string nodeName() override {
      return "NumberFloatExpression";
    }

    std::unique_ptr<ConstValueExpression> clone() const override {
      return make_unique<NumberFloatExpression>(*this);
    }
};

class StringExpression: public ConstValueExpression {
  public:
    string value;

    string nodeName() override {
      return "StringExpression";
    }

    std::unique_ptr<ConstValueExpression> clone() const override {
      return make_unique<StringExpression>(*this);
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

    std::unique_ptr<ConstValueExpression> clone() const override {
      return make_unique<BoolExpression>(*this);
    }
};



class CallExpressionArgument: public ASTNode {
  public:
    /** the value of the function argument at the specific call */
    unique_ptr<Expression> expression;

    /** contains the argument name if arg was used with that name like 'arg1=value' */
    optional<string> argName = nullopt;

    /** links to declaration of the function argument */
    FunctionParamDeclaration *argumentDeclaration;

    string nodeName() override {
      return "CallExpressionArgument";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({expression.get()});
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


    AstChildRange getChildNodes() override {
      return makeAstRange({}, {
        makeContainerIter_ValueToPtr<ASTNode>(argumentsNonNamed),
        makeContainerIter_ValueToPtr<ASTNode>(argumentsNamed)
      });
    }
};


/**
 * Expression like myObject.func()
 * '.func()' is a MemberExpression
 * and 'myObject' is parent of '.func()' and a VariableExpression
 */
class MemberCallExpression: public CallExpression {
  public:
    /** parent of the member has type IdentifierExpression */
    // @todo make type IdentifierExpression
    unique_ptr<Expression> parent; // IdentifierExpression

    string nodeName() override {
      return "MemberCallExpression";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({parent.get()}, {
          makeContainerIter_ValueToPtr<ASTNode>(argumentsNonNamed),
          makeContainerIter_ValueToPtr<ASTNode>(argumentsNamed)
      });
    }
};





/**************************************************************
 *** Statements
 */

// @todo make separate class for MemberVariableDeclaration
class AbstractVariableDeclaration {
  public:
    string name;
    string typeName;
    unique_ptr<LangType> type;
    bool isMutable = true;

    /** links to the allocated llvm value for the variable, when its a memberVariable this is null */
    llvm::Value *llvmVariable;

    /** links to the allocated ir value for the variable, when its a memberVariable this is null */
    IRValueVar *irVariablePtr = nullptr;

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

    AstChildRange getChildNodes() override
    { return makeAstRange({initExpression.get()}); }
};

class ReturnStatement: public Statement {
  public:
    /** the return value */
    optional<unique_ptr<Expression>> expression;
    unique_ptr<LangType> returnType;

    string nodeName() override {
      return "ReturnStatement";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({expression->get()});
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

    AstChildRange getChildNodes() override {
      return makeAstRange({}, {
          makeContainerIter_SmartPtrToPtr<ASTNode>(statements)
      });
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

    AstChildRange getChildNodes() override {
      if (elseBody)
        return makeAstRange({condition.get(), ifBody.get(), elseBody.get()});
      else
        return makeAstRange({condition.get(), ifBody.get()});
    }
};


class WhileStatement: public Statement {
  public:
    unique_ptr<Expression> condition;
    unique_ptr<CompoundStatement> body;

    string nodeName() override {
      return "WhileStatement";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({condition.get(), body.get()});
    }
};


class VariableAssignStatement: public Statement {
  public:
    unique_ptr<Expression> valueExpression;
    unique_ptr<VariableExpression> variableExpression;

    string nodeName() override {
      return "VariableAssignStatement";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({variableExpression.get(), valueExpression.get()});
    }
};



/**************************************************************
 *** Declarations
 */

class FunctionParamDeclaration: public ASTNode, public AbstractVariableDeclaration {
  public:
    /** optional */
    unique_ptr<Expression> defaultExpression; // is ConstValueExpression

    string nodeName() override {
      return "FunctionParamDeclaration";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({defaultExpression.get()});
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

    AstChildRange getChildNodes() override {
      return makeAstRange({body.get()}, {
          makeContainerIter_ValueToPtr<ASTNode>(arguments)
      });
    }
};



class ClassDeclaration: public ASTNode {
  public:
    string name;
    list<unique_ptr<VariableDeclaration>> variableDeclarations;
    unique_ptr<VariableDeclaration> thisVarDecl;
    list<FunctionDeclaration> functionDeclarations;
    unique_ptr<FunctionDeclaration> constructor = make_unique<FunctionDeclaration>();

    /** llvm type for this class */
    llvm::StructType *llvmStructType = nullptr;
    int llvmStructSizeBytes = -1;


    string nodeName() override {
      return "ClassDeclaration";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({constructor.get(), thisVarDecl.get()}, {
        makeContainerIter_SmartPtrToPtr<ASTNode>(variableDeclarations),
        makeContainerIter_ValueToPtr<ASTNode>(functionDeclarations)
      });
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


/**
 * Root node of the Ast.
 */
class RootDeclarations: public ASTNode {
  public:
    list<unique_ptr<VariableDeclaration>> variableDeclarations;
    list<FunctionDeclaration> functionDeclarations;
    list<unique_ptr<ClassDeclaration>> classDeclarations;
    FunctionDeclaration *mainFunction = nullptr;

    string nodeName() override {
      return "RootDeclarations";
    }

    AstChildRange getChildNodes() override {
      return makeAstRange({}, {
          makeContainerIter_SmartPtrToPtr<ASTNode>(classDeclarations),
          makeContainerIter_SmartPtrToPtr<ASTNode>(variableDeclarations),
          makeContainerIter_ValueToPtr<ASTNode>(functionDeclarations)
      });
    }
};




// LangTypes
string ClassType::toString()
{
  return "class_" + classDeclaration->name;
}