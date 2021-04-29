#pragma once
#include "ir/builder/exceptions.h"
#include "ir/IRInstructions.h"
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include "ir/builder/IRBuilder.h"

/**
 * Generate Intermediate representation from the AST.
 */
class IRGenerator: public AstVisitor<IRValueVar*, IRBuilder&> {
  public:
    IRModule module;

    IRGenerator() {
      globalVarInitValueHoldingFunc.basicBlocks.push_back(IRBasicBlock("entry"));
      globalVarInitValueHoldingBB = &globalVarInitValueHoldingFunc.basicBlocks.back();
    }

    void generate(RootDeclarations &rootDecls, string srcFileName){
      IRBuilder builder(module);
      builder.module.sourceFileName = move(srcFileName);
      accept(&rootDecls, builder);

      builder.module.isValid = false;
    }




  private:

    // pseudo function with entry bb for init values of global variables
    // TODO: clean workaround: pseudo function with entry bb for init values of global variables
    IRFunction globalVarInitValueHoldingFunc = IRFunction("globalVarInitValueHoldingFunc");
    IRBasicBlock *globalVarInitValueHoldingBB = nullptr;

    /// ********************************************************************
    /// Declarations

  private:
    IRValueVar* visitRootDecl(RootDeclarations *rootDecl, IRBuilder &builder) override {
      for (auto &decl: rootDecl->classDeclarations) {
        error("classes are not implemented yet in IR", decl->location);
      }
      for (auto &decl: rootDecl->variableDeclarations) {
        visitGlobalVariableDecl(decl.get(), builder);
      }
      for (auto &decl: rootDecl->functionDeclarations) {
        accept(&decl, builder);
      }
      return nullptr;
    }


    /// TODO: partially merge with visitVariableDecl
    void visitGlobalVariableDecl(VariableDeclaration *varDecl, IRBuilder &builder) {
      if (!varDecl->initExpression) {
        error("global variables need an initial value", varDecl->location);
        return;
      }

      builder.setInsertionBasicBlock(*globalVarInitValueHoldingBB);
      accept(varDecl->initExpression.get(), builder);
      module.globalVariablesInitValues.push_back(globalVarInitValueHoldingBB->instructions.back());
      IRValueVar *initVal = &module.globalVariablesInitValues.back();
      globalVarInitValueHoldingBB->instructions.pop_back();

      IRValueVar *valVar = nullptr;
      if (auto type = dynamic_cast<BuildInType*>(varDecl->type.get())) {
        IRGlobalVar &var = builder.GlobalVar(varDecl->name);
        var.type = IRTypePointer(langTypeToIRType(type));
        var.initValue = initVal;
        var.name = varDecl->name;
        valVar = (IRValueVar*)&var;
      }
      else {
        error("currently only buildIn types are supported (will ignore var decl)", varDecl->location);
      }
      varDecl->irVariablePtr = valVar;
    }


    IRValueVar* visitVariableDecl(VariableDeclaration *varDecl, IRBuilder &builder) override {
      IRValueVar *valVar = nullptr;
      if (auto type = dynamic_cast<BuildInType*>(varDecl->type.get())) {
        IRBuildInTypeAllocation &alloc = builder.Instruction(IRBuildInTypeAllocation(type->type));
        alloc.name = varDecl->name;
        valVar = (IRValueVar*)&alloc;
      }
      else {
        error("currently only buildIn types are supported (will ignore var decl)", varDecl->location);
      }

      if (!varDecl->initExpression) {
        throw IRGenException("variables need an initial value", varDecl->location);
      }

      IRValueVar *initVal = accept(varDecl->initExpression.get(), builder);
      builder.Instruction(IRStore(valVar, initVal));
      varDecl->irVariablePtr = valVar;

      return valVar;
    }


    IRValueVar *visitFunctionDecl(FunctionDeclaration *funcDecl, IRBuilder &builder) override {
      IRFunction &function = builder.Function(funcDecl->name);
      function.returnType = langTypeToIRType(funcDecl->returnType);
      function.isExtern = funcDecl->isExtern;

      for (auto &a : funcDecl->arguments) {
        accept(&a, builder);
      }

      if (funcDecl->body) {
        accept(funcDecl->body.get(), builder);
      }
      return nullptr;
    }


    IRValueVar* visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, IRBuilder &builder) override {
      if (funcParam->defaultExpression) {
        accept(funcParam->defaultExpression.get(), builder);
      }
    }

    IRValueVar* visitClassDecl(ClassDeclaration *classDecl, IRBuilder &builder) override {
      for (auto &e : classDecl->variableDeclarations) {
        accept(e.get(), builder);
      }
      for (auto &e : classDecl->functionDeclarations) {
        accept(&e, builder);
      }
    }



    /// ********************************************************************
    /// Statements

    IRValueVar* visitCompoundStatement(CompoundStatement *st, IRBuilder &builder) override {
      for (auto &child : st->statements) {
        accept(child.get(), builder);
      }
      return nullptr;
    }

    IRValueVar* visitVariableAssignStatement(VariableAssignStatement *st, IRBuilder &builder) override {
      accept(st->variableExpression.get(), builder);
      accept(st->valueExpression.get(), builder);
    }

    IRValueVar* visitReturnStatement(ReturnStatement *st, IRBuilder &builder) override {
      IRValueVar *returnValue = nullptr;
      if (st->expression) {
        returnValue = accept(st->expression->get(), builder);
      }
      return &(IRValueVar&) builder.Instruction(IRReturn(returnValue));
    }

    IRValueVar* visitIfStatement(IfStatement *st, IRBuilder &builder) override {
      accept(st->condition.get(), builder);
      accept(st->ifBody.get(), builder);
      if (st->elseBody) {
        accept(st->elseBody.get(), builder);
      }
    }

    IRValueVar* visitWhileStatement(WhileStatement *st, IRBuilder &builder) override {
      accept(st->condition.get(), builder);
      accept(st->body.get(), builder);
    }



    /// ********************************************************************
    /// Expressions
    IRValueVar *visitBoolExpression(BoolExpression *ex, IRBuilder &builder) override {
      auto &inst = builder.Instruction(IRConstBoolean());
      inst.value = ex->value;
      return (IRValueVar*)&inst;
    }

    IRValueVar* visitNumberIntExpression(NumberIntExpression *ex, IRBuilder &builder) override {
      auto &inst = builder.Instruction(IRConstNumberI32());
      inst.value = ex->value;
      return (IRValueVar*)&inst;
    }

    IRValueVar* visitNumberFloatExpression(NumberFloatExpression *ex, IRBuilder &builder) override {
    }

    IRValueVar* visitStringExpression(StringExpression *ex, IRBuilder &builder) override {
    }


    IRValueVar* visitVariableExpression(VariableExpression *ex, IRBuilder &builder) override {
      if (ex->variableDeclaration->irVariablePtr == nullptr) {
        error("no variable allocation generated, can't load variable", ex);
        return nullptr;
      }
      return &(IRValueVar&)builder.Instruction(IRLoad(ex->variableDeclaration->irVariablePtr));
    }

    IRValueVar* visitMemberVariableExpression(MemberVariableExpression *ex, IRBuilder &builder) override {
      accept(ex->parent.get(), builder);
    }


    IRValueVar* visitCallExpression(CallExpression *ex, IRBuilder &builder) override {
      for (auto &a : ex->argumentsNonNamed) {
        accept(&a, builder);
      }
    }

    IRValueVar* visitMemberCallExpression(MemberCallExpression *ex, IRBuilder &builder) override {
      accept(ex->parent.get(), builder);
      visitCallExpression(ex, builder);
    }

    IRValueVar* visitCallExpressionArgument(CallExpressionArgument *ex, IRBuilder &builder) override {
      accept(ex->expression.get(), builder);
    }


    IRValueVar* visitUnaryExpression(UnaryExpression *ex, IRBuilder &builder) override {
      accept(ex->innerExpression.get(), builder);
    }

    IRValueVar* visitBinaryExpression(BinaryExpression *ex, IRBuilder &builder) override {
      auto resultType = dynamic_cast<BuildInType*>(ex->resultType.get());
      auto operandType = dynamic_cast<BuildInType*>(ex->lhs->resultType.get());
      if (!resultType || !operandType) {
        throw IRGenException("only buildIn types are currently supported", ex->location);
      }

      auto lVal = accept(ex->lhs.get(), builder);
      auto rVal = accept(ex->rhs.get(), builder);

      // check value type and operation type
      // number calculation
      if (operandType->isNumericalType() && resultType->equals(operandType)) {
        auto &binOpInst = builder.Instruction(IRNumberCalculationBinary());
        binOpInst.type = IRTypeBuildIn(resultType->type);
        binOpInst.op = IR_NUMBER_CALCULATION_BINARY_OP_fromBinaryExpressionOp(ex->operation, ex);
        binOpInst.lhs = lVal;
        binOpInst.rhs = rVal;
        return (IRValueVar*)&binOpInst;
      }
      // number compare
      else if (operandType->isNumericalType() && resultType->isBooleanType()) {
        auto &binOpInst = builder.Instruction(IRNumberCompareBinary());
        binOpInst.type = IRTypeBuildIn(resultType->type);
        binOpInst.op = IR_NUMBER_COMPARE_BINARY_OP_fromBinaryExpressionOp(ex->operation, ex);
        binOpInst.lhs = lVal;
        binOpInst.rhs = rVal;
        return (IRValueVar*)&binOpInst;
      }
      else{
        warn("only numerical type is currently supported for binary calculation", ex->location);
      }
    }



    MsgScope error(const string& msg, SrcLocationRange &location) {
      return printError("ir generation", msg, location);
    }
    MsgScope error(const string& msg, ASTNode *node) {
      return error(msg, node->location);
    }

    MsgScope warn(const string& msg, SrcLocationRange &location) {
      return printWarn("ir generation", msg, location);
    }
    MsgScope warn(const string& msg, ASTNode *node) {
      return warn(msg, node->location);
    }
};



