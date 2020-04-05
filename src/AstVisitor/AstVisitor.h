#pragma once

#include "parser/AST.h"
#include<iostream>
using namespace std;


/*
 * AstVisitor class
 */
template <typename RESULT = void, typename ARG = void*>
class AstVisitor
{
  public:
    /// Declarations

    virtual RESULT visitRootDecl(RootDeclarations *rootDecl, ARG arg) {}

    virtual RESULT visitVariableDecl(VariableDeclaration *varDecl, ARG arg) {}

    virtual RESULT visitFunctionDecl(FunctionDeclaration *funcDecl, ARG arg) {}
    virtual RESULT visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, ARG arg) {}

    virtual RESULT visitClassDecl(ClassDeclaration *classDecl, ARG arg) {}


    /// Statements
    virtual RESULT visitCompoundStatement(CompoundStatement *st, ARG arg) {}
    virtual RESULT visitVariableAssignStatement(VariableAssignStatement *st, ARG arg) {}
    virtual RESULT visitReturnStatement(ReturnStatement *st, ARG arg) {}
    virtual RESULT visitIfStatement(IfStatement *st, ARG arg) {}
    virtual RESULT visitWhileStatement(WhileStatement *st, ARG arg) {}


    /// Expressions
    virtual RESULT visitBoolExpression(BoolExpression *ex, ARG arg) {}
    virtual RESULT visitNumberIntExpression(NumberIntExpression *ex, ARG arg) {}
    virtual RESULT visitNumberFloatExpression(NumberFloatExpression *ex, ARG arg) {}
    virtual RESULT visitStringExpression(StringExpression *ex, ARG arg) {}

    virtual RESULT visitVariableExpression(VariableExpression *ex, ARG arg) {}
    virtual RESULT visitMemberVariableExpression(MemberVariableExpression *ex, ARG arg) {}

    virtual RESULT visitCallExpression(CallExpression *ex, ARG arg) {}
    virtual RESULT visitMemberCallExpression(MemberCallExpression *ex, ARG arg) {}
    virtual RESULT visitCallExpressionArgument(CallExpressionArgument *ex, ARG arg) {}

    virtual RESULT visitUnaryExpression(UnaryExpression *ex, ARG arg) {}
    virtual RESULT visitBinaryExpression(BinaryExpression *ex, ARG arg) {}


    /**
     * When accept could not find matching function for node.
     */
    virtual RESULT visitUnknownNode(ASTNode *node, ARG arg) {}

    /**
     * Called before each specific visit method is called
     */
    virtual void beforeEachVisit(ASTNode *node, ARG arg) {}


    RESULT accept(ASTNode *node, ARG arg = nullptr) {
      if (node) {
        beforeEachVisit(node, arg);
      }

      // Declarations
      if (auto n = dynamic_cast<RootDeclarations*>(node)) {
        return visitRootDecl(n, arg);
      }
      else if (auto n = dynamic_cast<VariableDeclaration*>(node)) {
        return visitVariableDecl(n, arg);
      }
      else if (auto n = dynamic_cast<FunctionDeclaration*>(node)) {
        return visitFunctionDecl(n, arg);
      }
      else if (auto n = dynamic_cast<FunctionParamDeclaration*>(node)) {
        return visitFunctionParamDeclaration(n, arg);
      }
      else if (auto n = dynamic_cast<ClassDeclaration*>(node)) {
        return visitClassDecl(n, arg);
      }

      // Statements
      else if (auto n = dynamic_cast<CompoundStatement*>(node)) {
        return visitCompoundStatement(n, arg);
      }
      else if (auto n = dynamic_cast<VariableAssignStatement*>(node)) {
        return visitVariableAssignStatement(n, arg);
      }
      else if (auto n = dynamic_cast<ReturnStatement*>(node)) {
        return visitReturnStatement(n, arg);
      }
      else if (auto n = dynamic_cast<IfStatement*>(node)) {
        return visitIfStatement(n, arg);
      }
      else if (auto n = dynamic_cast<WhileStatement*>(node)) {
        return visitWhileStatement(n, arg);
      }

      // Expressions
      else if (auto n = dynamic_cast<BoolExpression*>(node)) {
        return visitBoolExpression(n, arg);
      }
      else if (auto n = dynamic_cast<NumberIntExpression*>(node)) {
        return visitNumberIntExpression(n, arg);
      }
      else if (auto n = dynamic_cast<NumberFloatExpression*>(node)) {
        return visitNumberFloatExpression(n, arg);
      }
      else if (auto n = dynamic_cast<StringExpression*>(node)) {
        return visitStringExpression(n, arg);
      }

      else if (auto n = dynamic_cast<MemberVariableExpression*>(node)) {
        return visitMemberVariableExpression(n, arg);
      }
      else if (auto n = dynamic_cast<VariableExpression*>(node)) {
        return visitVariableExpression(n, arg);
      }

      else if (auto n = dynamic_cast<MemberCallExpression*>(node)) {
        return visitMemberCallExpression(n, arg);
      }
      else if (auto n = dynamic_cast<CallExpression*>(node)) {
        return visitCallExpression(n, arg);
      }
      else if (auto n = dynamic_cast<CallExpressionArgument*>(node)) {
        return visitCallExpressionArgument(n, arg);
      }

      else if (auto n = dynamic_cast<UnaryExpression*>(node)) {
        return visitUnaryExpression(n, arg);
      }
      else if (auto n = dynamic_cast<BinaryExpression*>(node)) {
        return visitBinaryExpression(n, arg);
      }

      return visitUnknownNode(node, arg);
    }
};


