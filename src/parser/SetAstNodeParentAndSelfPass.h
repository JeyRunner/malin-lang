#pragma once
#include "AST.h"
#include "AstVisitor/AstVisitorTemplate.h"


#include<iostream>
using namespace std;


struct Arg {
    ASTNode *parent = nullptr;
    unique_ptr<Expression> *selfExpression = nullptr;
    unique_ptr<VariableExpression> *selfVariableExpression= nullptr;
    unique_ptr<Statement> *selfStatement = nullptr;
};


/*
 * Set parentAstNode prop and self of each ast node.
 */
class SetAstNodeParentAndSelfPass: AstVisitor<void, Arg>
{
  public:
    void run(RootDeclarations &root) {
      accept(&root, Arg{.parent=nullptr});

      // verify
      // check if all nodes have parent and self
      verify(&root);
    }

    void verify(ASTNode *node) {
      string error = "";
      if (!node->parentAstNode)
        error+= "has no parentAstNode";
      if (auto ex = dynamic_cast<Expression*>(node)) {
        if (!ex->selfSet())
          error+= "    has no self";
      }
      if (!error.empty()) {
        cout << node->nodeName() << " (" << node->location.toString() << ") : " << error << endl;
      }
      for (auto child : node->getChildNodes()) {
        verify(child);
      }
    }


  private:
    void beforeEachVisit(ASTNode *node, Arg arg) override {
      // set parent
      node->parentAstNode = arg.parent;
      // when its an expression set self
      if (auto ex = dynamic_cast<Expression*>(node)) {
        ex->self1 = arg.selfExpression;
        ex->self2 = arg.selfStatement;
        ex->self3 = arg.selfVariableExpression;
      }
    }



    /// ********************************************************************
    /// Declarations

    void visitRootDecl(RootDeclarations *rootDecl, Arg arg) override {
      for (auto &decl: rootDecl->classDeclarations) {
        accept(decl.get(), Arg{.parent=rootDecl});
      }
      for (auto &decl: rootDecl->variableDeclarations) {
        accept(decl.get(), Arg{.parent=rootDecl});
      }
      for (auto &decl: rootDecl->functionDeclarations) {
        accept(&decl, Arg{.parent=rootDecl});
      }
    }

    void visitVariableDecl(VariableDeclaration *varDecl, Arg arg) override {
      if (varDecl->initExpression) {
        accept(varDecl->initExpression.get(), Arg{.parent=varDecl, .selfExpression=&varDecl->initExpression});
      }
    }

    void visitFunctionDecl(FunctionDeclaration *funcDecl, Arg arg) override {
      for (auto &a : funcDecl->arguments) {
        accept(&a, Arg{.parent=funcDecl});
      }
      accept(funcDecl->body.get(), Arg{.parent=funcDecl});
    }

    void visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, Arg arg) override {
      if (funcParam->defaultExpression) {
        accept(funcParam->defaultExpression.get(), Arg{.parent=funcParam, .selfExpression=&funcParam->defaultExpression});
      }
    }

    void visitClassDecl(ClassDeclaration *classDecl, Arg arg) override {
      accept(classDecl->constructor.get(), Arg{.parent=classDecl});
      if (classDecl->thisVarDecl) {
        // @todo this is added after this pass in decorator, thus thisVarDecl has no parent
        accept(classDecl->thisVarDecl.get(), Arg{.parent=classDecl});
      }
      for (auto &e : classDecl->variableDeclarations) {
        accept(e.get(), Arg{.parent=classDecl});
      }
      for (auto &e : classDecl->functionDeclarations) {
        accept(&e, Arg{.parent=classDecl});
      }
    }



    /// ********************************************************************
    /// Statements

    void visitCompoundStatement(CompoundStatement *st, Arg arg) override {
      for (auto &child : st->statements) {
        accept(child.get(), Arg{.parent=st, .selfStatement=&child});
      }
    }

    void visitVariableAssignStatement(VariableAssignStatement *st, Arg arg) override {
      accept(st->variableExpression.get(), Arg{.parent=st, .selfVariableExpression=&st->variableExpression});
      accept(st->valueExpression.get(), Arg{.parent=st, .selfExpression=&st->valueExpression});
    }

    void visitReturnStatement(ReturnStatement *st, Arg arg) override {
      if (st->expression) {
        accept(st->expression->get(), Arg{.parent=st, .selfExpression=&*st->expression});
      }
    }

    void visitIfStatement(IfStatement *st, Arg arg) override {
      accept(st->condition.get(), Arg{.parent=st, .selfExpression=&st->condition});
      accept(st->ifBody.get(), Arg{.parent=st});
      if (st->elseBody) {
        accept(st->elseBody.get(), Arg{.parent=st});
      }
    }

    void visitWhileStatement(WhileStatement *st, Arg arg) override {
      accept(st->condition.get(), Arg{.parent=st, .selfExpression=&st->condition});
      accept(st->body.get(), Arg{.parent=st});
    }



    /// ********************************************************************
    /// Expressions

    void visitNumberIntExpression(NumberIntExpression *ex, Arg arg) override {
    }

    void visitNumberFloatExpression(NumberFloatExpression *ex, Arg arg) override {
    }

    void visitStringExpression(StringExpression *ex, Arg arg) override {
    }


    void visitVariableExpression(VariableExpression *ex, Arg arg) override {
    }

    void visitMemberVariableExpression(MemberVariableExpression *ex, Arg arg) override {
      accept(ex->parent.get(), Arg{.parent=ex, .selfExpression=&ex->parent});
    }


    void visitCallExpression(CallExpression *ex, Arg arg) override {
      for (auto &a : ex->argumentsNonNamed) {
        accept(&a, Arg{.parent=ex});
      }
      for (auto &a : ex->argumentsNamed) {
        accept(&a, Arg{.parent=ex});
      }
    }

    void visitMemberCallExpression(MemberCallExpression *ex, Arg arg) override {
      accept(ex->parent.get(), Arg{.parent=ex, .selfExpression=&ex->parent});
      visitCallExpression(ex, arg);
    }

    void visitCallExpressionArgument(CallExpressionArgument *ex, Arg arg) override {
      accept(ex->expression.get(), Arg{.parent=ex, .selfExpression=&ex->expression});
    }


    void visitUnaryExpression(UnaryExpression *ex, Arg arg) override {
      accept(ex->innerExpression.get(), Arg{.parent=ex, .selfExpression=&ex->innerExpression});
    }

    void visitBinaryExpression(BinaryExpression *ex, Arg arg) override {
      accept(ex->lhs.get(), Arg{.parent=ex, .selfExpression=&ex->lhs});
      accept(ex->rhs.get(), Arg{.parent=ex, .selfExpression=&ex->rhs});
    }
};


