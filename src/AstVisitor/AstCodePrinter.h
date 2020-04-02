#pragma once

#include<iostream>
#include "AstVisitor.h"
using namespace std;



/**
 * Print the ast as code.
 */
class AstCodePrinter: AstVisitor<void, int>
{
  public:
    string getAstAsCode(RootDeclarations &root) {
      accept(&root, 0);
      return os.str();
    }

  private:
    ostringstream os;

    void osInd(int depth = 0) {
      for (int i = 0; i < depth; ++i) {
        os << "  ";
      }
    }



    /// ********************************************************************
    /// Declarations

    void visitRootDecl(RootDeclarations *rootDecl, int depth) override {
      for (auto &decl: rootDecl->classDeclarations) {
        accept(decl.get(), 0);
        os << endl << endl << endl;
      }
      for (auto &decl: rootDecl->variableDeclarations) {
        accept(decl.get(), 0);
        osInd();
        os << ";";
      }
      os << endl << endl;
      for (auto &decl: rootDecl->functionDeclarations) {
        accept(&decl, 0);
        os << endl << endl;
      }
    }

    void visitVariableDecl(VariableDeclaration *varDecl, int depth) override {
      if (!varDecl->isMemberVariable()) {
        os << "let ";
      }
      os << varDecl->name;
      os << ": " << varDecl->type->toString();
      if (varDecl->initExpression) {
        os << " = ";
        accept(varDecl->initExpression.get(), 0);
      }
    }

    void visitFunctionDecl(FunctionDeclaration *funcDecl, int depth) override {
      osInd();
      os << "fun ";
      if (funcDecl->isExtern) {
        os << "extern ";
      }
      os << funcDecl->name << "(";
      // args
      int i = 0;
      for (auto &a : funcDecl->arguments) {
        accept(&a, depth);
        if (i < funcDecl->arguments.size()-1) {
          os << ", ";
        }
        i++;
      }
      os << "): " << funcDecl->returnType->toString() << " ";
      accept(funcDecl->body.get(), depth);
    }

    void visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, int depth) override {
      os << funcParam->name;
      os << ": " << funcParam->type->toString();
      if (funcParam->defaultExpression) {
        os << " = ";
        accept(funcParam->defaultExpression.get(), depth);
      }
    }

    void visitClassDecl(ClassDeclaration *classDecl, int depth) override {
      os << "class " << classDecl->name;
      os <<" {" << endl;
      for (auto &e : classDecl->variableDeclarations) {
        osInd(depth+1);
        accept(e.get(), depth+1);
        os << ";" << endl;
      }
      for (auto &e : classDecl->functionDeclarations) {
        os << endl;
        osInd(depth+1);
        accept(&e, depth+1);
        os << endl;
      }
      os <<"}";
    }



    /// ********************************************************************
    /// Statements

    void visitCompoundStatement(CompoundStatement *st, int depth) override {
      os << "{" << endl;
      for (auto &child : st->statements) {
        osInd(depth+1);
        accept(child.get(), depth+1);
        os << ";" << endl;
      }
      osInd(depth);
      os << "}";
    }

    void visitVariableAssignStatement(VariableAssignStatement *st, int depth) override {
      accept(st->variableExpression.get(), depth);
      os << " = ";
      accept(st->valueExpression.get(), depth);
    }

    void visitReturnStatement(ReturnStatement *st, int depth) override {
      os << "return";
      if (st->expression) {
        os << " ";
        accept(st->expression->get(), depth);
      }
    }

    void visitIfStatement(IfStatement *st, int depth) override {
      os << "if (";
      accept(st->condition.get(), depth);
      os << ") ";
      accept(st->ifBody.get(), depth);
      if (st->elseBody) {
        os << endl;
        osInd(depth);
        os << "else ";
        accept(st->elseBody.get(), depth);
      }
    }

    void visitWhileStatement(WhileStatement *st, int depth) override {
      os << "while (";
      accept(st->condition.get(), depth);
      os << ")";
      accept(st->body.get(), depth);
    }



    /// ********************************************************************
    /// Expressions

    void visitNumberIntExpression(NumberIntExpression *ex, int depth) override {
      os << ex->value;
    }

    void visitNumberFloatExpression(NumberFloatExpression *ex, int depth) override {
      os << ex->value;
    }

    void visitStringExpression(StringExpression *ex, int depth) override {
      os << ex->value;
    }


    void visitVariableExpression(VariableExpression *ex, int depth) override {
      os << ex->variableDeclaration->name;
    }

    void visitMemberVariableExpression(MemberVariableExpression *ex, int depth) override {
      accept(ex->parent.get(), depth);
      os << "." << ex->variableDeclaration->name;
    }


    void visitCallExpression(CallExpression *ex, int depth) override {
      os << ex->functionDeclaration->name;
      os << "(";
      int i = 0;
      for (auto &a : ex->argumentsNonNamed) {
        accept(&a, depth);
        if (i < ex->argumentsNonNamed.size()-1) {
          os << ", ";
        }
        i++;
      }
      os << ")";
    }

    void visitMemberCallExpression(MemberCallExpression *ex, int depth) override {
      accept(ex->parent.get(), depth);
      os << ".";
      visitCallExpression(ex, depth);
    }

    void visitCallExpressionArgument(CallExpressionArgument *ex, int depth) override {
      os << ex->argumentDeclaration->name << "=";
      accept(ex->expression.get(), depth);
    }


    void visitUnaryExpression(UnaryExpression *ex, int depth) override {
      os << "(" << toString(ex->operation);
      accept(ex->innerExpression.get(), depth);
      os << ")";
    }

    void visitBinaryExpression(BinaryExpression *ex, int depth) override {
      os << "(";
      accept(ex->lhs.get(), depth);
      os << " " << toString(ex->operation) << " ";
      accept(ex->rhs.get(), depth);
      os << ")";
    }


    void visitUnknownNode(ASTNode *ex, int arg) override {
      if (!ex) {
        return;
      }
      os << "UNKNOWN_AST_NODE";
    }
};


