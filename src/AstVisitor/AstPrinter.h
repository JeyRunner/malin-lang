#pragma once

#include<iostream>
#include "AstVisitor.h"
using namespace std;



/**
 * Print the ast as tree.
 */
class AstPrinter: AstVisitor<void, int>
{
  public:
    /**
     * Print the Ast tree to a stream.
     * @param os print the tree in this stream
     * @param printLocation add the file location to each ast node
     */
    AstPrinter(ostream &os, bool printLocation = false) : os(os), printLocationEnabled(printLocation)
    {
    }

    void printTree(RootDeclarations &root) {
      accept(&root, 0);
    }

  private:
    ostream &os;
    bool printLocationEnabled;

    void osInd(int depth = 0) {
      for (int i = 0; i < depth; ++i) {
        os << "  ";
      }
    }

    void nextLine(int depth = 0) {
      os << endl;
      osInd(depth);
    }

    void printLocation(ASTNode *node) {
      if (printLocationEnabled) {
        os << "   location: " << termcolor::reset << node->location.toString() << termcolor::reset;
      }
    }

    void printAttribute(string name, int parentDepth) {
      nextLine(parentDepth);
      os << "+- " << name << ": ";
    }

    void printAttribute(string name, string value, int parentDepth) {
      printAttribute(move(name), parentDepth);
      os << value;
    }
    
    void printSubNode(ASTNode *node, int parentDepth) {
      accept(node, parentDepth+2);
    }

    void printType(unique_ptr<LangType> &type, int parentDepth) {
      if (type)
        printAttribute("type", type->toString(), parentDepth);
    }



    /**
     * Print name of the node
     */
    void beforeEachVisit(ASTNode *node, int depth) override {
      nextLine(depth);
      os << termcolor::underline;
      if (dynamic_cast<Expression*>(node)) {
        os << termcolor::yellow << node->nodeName() << termcolor::reset;
      }
      else if (dynamic_cast<Statement*>(node)) {
        os << termcolor::green << node->nodeName() << termcolor::reset;
      }
      else {
        os << termcolor::cyan << node->nodeName() << termcolor::reset;
      }
    }


    /// ********************************************************************
    /// Declarations

    void visitRootDecl(RootDeclarations *rootDecl, int depth) override {
      printAttribute("classes", depth);
      for (auto &decl: rootDecl->classDeclarations) {
        printSubNode(decl.get(), depth);
      }
      printAttribute("globalVariables", depth);
      for (auto &decl: rootDecl->variableDeclarations) {
        printSubNode(decl.get(), depth);
      }
      printAttribute("functions", depth);
      for (auto &decl: rootDecl->functionDeclarations) {
        printSubNode(&decl, depth);
      }
      os << endl;
    }

    void visitVariableDecl(VariableDeclaration *varDecl, int depth) override {
      os << "(name: " << varDecl->name << ", typeName: " << varDecl->typeName << ", isMemberVariable: " << toString(varDecl->isMemberVariable());
      if (varDecl->isMemberVariable())
        os << ", memberIndex: " << varDecl->memberIndex;
      os << ", mutable: "<< toString(varDecl->isMutable) <<")";
      printLocation(varDecl);
      if (varDecl->type) {
        printAttribute("type", depth);
        os << varDecl->type->toString();
      }

      if (varDecl->initExpression) {
        printAttribute("init", depth);
        printSubNode(varDecl->initExpression.get(), depth);
      }
    }

    void visitFunctionDecl(FunctionDeclaration *funcDecl, int depth) override {
      os << "(name: " << funcDecl->name << ", typeName: " << funcDecl->typeName << ")";
      printLocation(funcDecl);
      if (funcDecl->returnType) {
        printAttribute("returnType", depth);
        os << funcDecl->returnType->toString();
      }
      if (!funcDecl->arguments.empty()) {
        printAttribute("arguments", depth);
        for (auto &arg : funcDecl->arguments)
          printSubNode(&arg, depth);
      }
      if (funcDecl->body) {
        printAttribute("body", depth);
        printSubNode(funcDecl->body.get(), depth);
      }
    }

    void visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, int depth) override {
      os << "(name: " << funcParam->name << ", typeName: " << funcParam->typeName << ", mutable: "<< toString(funcParam->isMutable) <<")";
      printLocation(funcParam);
      if (funcParam->type) {
        printAttribute("type", depth);
        os << funcParam->type->toString();
      }

      if (funcParam->defaultExpression) {
        printAttribute("defaultExpression", depth);
        printSubNode(funcParam->defaultExpression.get(), depth);
      }
    }

    void visitClassDecl(ClassDeclaration *classDecl, int depth) override {
      os << "(name: " << classDecl->name << ")";
      printLocation(classDecl);
      printAttribute("memberVariables", depth);
      for (auto &e : classDecl->variableDeclarations) {
        printSubNode(e.get(), depth);
      }
      printAttribute("memberFunctions", depth);
      for (auto &e : classDecl->functionDeclarations) {
        printSubNode(&e, depth);
      }
    }



    /// ********************************************************************
    /// Statements

    void visitCompoundStatement(CompoundStatement *st, int depth) override {
      printLocation(st);
      printAttribute("statements", depth);
      for (auto &child : st->statements) {
        printSubNode(child.get(), depth);
      }
    }

    void visitVariableAssignStatement(VariableAssignStatement *st, int depth) override {
      printLocation(st);
      printAttribute("variable", depth);
      printSubNode(st->variableExpression.get(), depth);
      printAttribute("value", depth);
      printSubNode(st->valueExpression.get(), depth);
    }

    void visitReturnStatement(ReturnStatement *st, int depth) override {
      printLocation(st);
      if (st->returnType) {
        printAttribute("type", depth);
        os << st->returnType->toString();
      }
      if (st->expression) {
        printAttribute("expression", depth);
        printSubNode(st->expression->get(), depth);
      }
    }

    void visitIfStatement(IfStatement *st, int depth) override {
      printLocation(st);
      printAttribute("condition", depth);
      printSubNode(st->condition.get(), depth);
      printAttribute("ifBody", depth);
      printSubNode(st->ifBody.get(), depth);
      if (st->elseBody) {
        printAttribute("elseBody", depth);
        printSubNode(st->elseBody.get(), depth);
      }
    }

    void visitWhileStatement(WhileStatement *st, int depth) override {
      printLocation(st);
      printAttribute("condition", depth);
      printSubNode(st->condition.get(), depth);
      printAttribute("body", depth);
      printSubNode(st->body.get(), depth);
    }



    /// ********************************************************************
    /// Expressions

    void visitBoolExpression(BoolExpression *ex, int depth) override {
      os << "(value: " << toString(ex->value) << ")";
      printType(ex->resultType, depth);
    }

    void visitNumberIntExpression(NumberIntExpression *ex, int depth) override {
      os << "(value: " << ex->value << ")";
      printType(ex->resultType, depth);
    }

    void visitNumberFloatExpression(NumberFloatExpression *ex, int depth) override {
      os << "(value: " << ex->value << ")";
      printType(ex->resultType, depth);
    }

    void visitStringExpression(StringExpression *ex, int depth) override {
      os << "(value: " << ex->value << ")";
      printType(ex->resultType, depth);
    }


    void visitVariableExpression(VariableExpression *ex, int depth) override {
      os << "(name: " << ex->name;
      if (ex->variableDeclaration)
        os << ", varDeclName: "<< ex->variableDeclaration->name;
      os << ")";
      printType(ex->resultType, depth);
    }

    void visitMemberVariableExpression(MemberVariableExpression *ex, int depth) override {
      os << "(name: " << ex->name;
      if (ex->variableDeclaration)
        os << ", varDeclName: "<< ex->variableDeclaration->name;
      os << ")";
      printType(ex->resultType, depth);
      printAttribute("parentExpr", depth);
      printSubNode(ex->parent.get(), depth);
    }


    void visitCallExpression(CallExpression *ex, int depth) override {
      os << "(calledName: " << ex->calledName << ")";
      printLocation(ex);
      printType(ex->resultType, depth);
      if (ex->functionDeclaration) {
        printAttribute("functionDeclarationName", ex->functionDeclaration->name, depth);
      }
      if (!ex->argumentsNonNamed.empty()) {
        printAttribute("arguments", depth);
        for (auto &a : ex->argumentsNonNamed)
          printSubNode(&a, depth);
      }
      if (!ex->argumentsNamed.empty()) {
        printAttribute("argumentsNamed", depth);
        for (auto &a : ex->argumentsNamed)
          printSubNode(&a, depth);
      }
    }

    void visitMemberCallExpression(MemberCallExpression *ex, int depth) override {
      visitCallExpression(ex, depth);
      printAttribute("parentExpr:", depth);
      printSubNode(ex->parent.get(), depth);
    }

    void visitCallExpressionArgument(CallExpressionArgument *ex, int depth) override {
      os << "(";
      if (ex->argName) {
        os << "NAMED, argName: " << *ex->argName;
      }
      else
        os << "UNNAMED";
      os << ")";
      if (ex->argumentDeclaration) {
        printAttribute("argumentDeclarationName", ex->argumentDeclaration->name, depth);
      }
      printSubNode(ex->expression.get(), depth);
    }


    void visitUnaryExpression(UnaryExpression *ex, int depth) override {
      os << "(operation: " << termcolor::bold << toString(ex->operation) << termcolor::reset << ")";
      printType(ex->resultType, depth);
      printAttribute("innerExpr", depth);
      printSubNode(ex->innerExpression.get(), depth);
    }

    void visitBinaryExpression(BinaryExpression *ex, int depth) override {
      os << "(operation: " << termcolor::bold << toString(ex->operation) << termcolor::reset << ")";
      printType(ex->resultType, depth);
      printAttribute("lhs", depth);
      printSubNode(ex->lhs.get(), depth);
      printAttribute("rhs", depth);
      printSubNode(ex->rhs.get(), depth);
    }


    void visitUnknownNode(ASTNode *ex, int arg) override {
      if (!ex) {
        return;
      }
      os << "(UNKNOWN_AST_NODE)";
    }
};


