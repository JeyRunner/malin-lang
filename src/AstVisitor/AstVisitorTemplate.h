#pragma once

#include<iostream>
#include "AstVisitor.h"
using namespace std;



/**
 * Template for recursive AstVisitor.
 */
template <typename RESULT = void, typename A = void*>
class AstVisitorTemplate: public AstVisitor<RESULT, A>
{
  public:


  private:

    /// ********************************************************************
    /// Declarations

    void visitRootDecl(RootDeclarations *rootDecl, A arg) override {
      for (auto &decl: rootDecl->classDeclarations) {
        accept(decl.get(), arg);
      }
      for (auto &decl: rootDecl->variableDeclarations) {
        accept(decl.get(), arg);
      }
      for (auto &decl: rootDecl->functionDeclarations) {
        accept(&decl, arg);
      }
    }

    void visitVariableDecl(VariableDeclaration *varDecl, A arg) override {
      if (varDecl->initExpression) {
        accept(varDecl->initExpression.get(), arg);
      }
    }

    void visitFunctionDecl(FunctionDeclaration *funcDecl, A arg) override {
      for (auto &a : funcDecl->arguments) {
        accept(&a, arg);
      }
      accept(funcDecl->body.get(), arg);
    }

    void visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, A arg) override {
      if (funcParam->defaultExpression) {
        accept(funcParam->defaultExpression.get(), arg);
      }
    }

    void visitClassDecl(ClassDeclaration *classDecl, A arg) override {
      for (auto &e : classDecl->variableDeclarations) {
        accept(e.get(), arg);
      }
      for (auto &e : classDecl->functionDeclarations) {
        accept(&e, arg);
      }
    }



    /// ********************************************************************
    /// Statements

    void visitCompoundStatement(CompoundStatement *st, A arg) override {
      for (auto &child : st->statements) {
        accept(child.get(), arg);
      }
    }

    void visitVariableAssignStatement(VariableAssignStatement *st, A arg) override {
      accept(st->variableExpression.get(), arg);
      accept(st->valueExpression.get(), arg);
    }

    void visitReturnStatement(ReturnStatement *st, A arg) override {
      if (st->expression) {
        accept(st->expression->get(), arg);
      }
    }

    void visitIfStatement(IfStatement *st, A arg) override {
      accept(st->condition.get(), arg);
      accept(st->ifBody.get(), arg);
      if (st->elseBody) {
        accept(st->elseBody.get(), arg);
      }
    }

    void visitWhileStatement(WhileStatement *st, A arg) override {
      accept(st->condition.get(), arg);
      accept(st->body.get(), arg);
    }



    /// ********************************************************************
    /// Expressions

    void visitNumberIntExpression(NumberIntExpression *ex, A arg) override {
    }

    void visitNumberFloatExpression(NumberFloatExpression *ex, A arg) override {
    }

    void visitStringExpression(StringExpression *ex, A arg) override {
    }


    void visitVariableExpression(VariableExpression *ex, A arg) override {
    }

    void visitMemberVariableExpression(MemberVariableExpression *ex, A arg) override {
      accept(ex->parent.get(), arg);
    }


    void visitCallExpression(CallExpression *ex, A arg) override {
      for (auto &a : ex->argumentsNonNamed) {
        accept(&a, arg);
      }
    }

    void visitMemberCallExpression(MemberCallExpression *ex, A arg) override {
      accept(ex->parent.get(), arg);
      visitCallExpression(ex, arg);
    }

    void visitCallExpressionArgument(CallExpressionArgument *ex, A arg) override {
      accept(ex->expression.get(), arg);
    }


    void visitUnaryExpression(UnaryExpression *ex, A arg) override {
      accept(ex->innerExpression.get(), arg);
    }

    void visitBinaryExpression(BinaryExpression *ex, A arg) override {
      accept(ex->lhs.get(), arg);
      accept(ex->rhs.get(), arg);
    }
};


