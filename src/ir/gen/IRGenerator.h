#pragma once
#include "ir/builder/exceptions.h"
#include "ir/IRInstructions.h"
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include "ir/builder/IRBuilder.h"
#include "IRGenHelper.h"
#include "ir/visitor/IRVisitor.h"
#include "ir/printer/IRPrinter.h"
#include "ir/passes/IRRemoveBBRedundantTermPass.hpp"


struct IRGenFlags {
    bool returnPointerValue = false;
    int functionArgumentIndex = -1;

    IRGenFlags withReturnPointerValue(bool v) {
      IRGenFlags copy = *this;
      copy.returnPointerValue = v;
      return copy;
    }

    IRGenFlags withFunctionArgumentIndexe(int v) {
      IRGenFlags copy = *this;
      copy.functionArgumentIndex = v;
      return copy;
    }
};


/**
 * Generate Intermediate representation from the AST.
 */
  class IRGenerator: public AstVisitor<IRValueVar*, IRGenFlags> {
  private:


  public:
    IRModule module;
    IRBuilder builder;

    IRGenerator(): builder(module) {
      globalVarInitValueHoldingFunc.basicBlocks.push_back(IRBasicBlock("entry"));
      globalVarInitValueHoldingBB = &globalVarInitValueHoldingFunc.basicBlocks.back();
    }

    void generate(RootDeclarations &rootDecls, string srcFileName){
      builder.module->sourceFileName = move(srcFileName);
      accept(&rootDecls, {});

      // cleanup instructions after termination instruction of a basic block
      IRRemoveBBRedundantTermPass removeBBRedundantTermPass;
      removeBBRedundantTermPass.run(module);

      builder.module->isValid = false;
    }




  private:

    /**
     * pseudo function with entry bb for init values of global variables
     * and default values of function arguments.
     */
    // TODO: clean workaround: pseudo function with entry bb for init values of global variables
    IRFunction globalVarInitValueHoldingFunc = IRFunction("globalVarInitValueHoldingFunc");
    IRBasicBlock *globalVarInitValueHoldingBB = nullptr;

    /// ********************************************************************
    /// Declarations

  private:
    IRValueVar* visitRootDecl(RootDeclarations *rootDecl, IRGenFlags flags) override {
      // gen global symbols
      // gen global variables [only definition]
      for (auto &global : rootDecl->variableDeclarations) {
        genGlobalVariableDefinition(global.get());
      }

      // gen function [only definition, no body]
      for (auto &function : rootDecl->functionDeclarations) {
        genFunctionDefinition(&function);
      }

      for (auto &decl: rootDecl->classDeclarations) {
        error("classes are not implemented yet in IR", decl->location);
      }
      for (auto &decl: rootDecl->variableDeclarations) {
        visitGlobalVariableDecl(decl.get(), flags);
      }
      for (auto &decl: rootDecl->functionDeclarations) {
        accept(&decl, flags);
      }
      return nullptr;
    }


    /**
     * Generate global variable definition without init.
     */
    void genGlobalVariableDefinition(VariableDeclaration *varDecl) {
      auto* ex = dynamic_cast<NumberExpression*>(varDecl->initExpression.get());
      if (!ex) {
        printError("", "globals need to have a constant init expression -> global ignored", varDecl->location);
        return;
      }
      if (!varDecl->initExpression) {
        error("global variables need an initial value", varDecl->location);
        return;
      }

      IRValueVar *valVar = nullptr;
      if (auto type = dynamic_cast<BuildInType*>(varDecl->type.get())) {
        IRGlobalVar &var = builder.GlobalVar(varDecl->name);
        var.type = IRTypePointer(langTypeToIRType(type));
        var.initValue = nullptr; // will be set later
        var.name = varDecl->name;
        valVar = (IRValueVar*)&var;
      }
      else {
        error("currently only buildIn types are supported (will ignore var decl)", varDecl->location);
      }
      varDecl->irVariablePtr = valVar;
    }


    /**
     * Generate function definition without body.
     */
    void genFunctionDefinition(FunctionDeclaration *funcDecl) {
      IRFunction &function = builder.Function(funcDecl->name);
      function.returnType = langTypeToIRType(funcDecl->returnType);
      function.isExtern = funcDecl->isExtern;
      funcDecl->irFunction = &function;

      for (auto &a : funcDecl->arguments) {
        accept(&a, {});
      }
      // fix references to the arguments from the ast nodes
      // this is needed due to resizing of the function.arguments vector pointers to its elements are not valid any more
      for (int i=0; auto &a : funcDecl->arguments) {
        a.irVariablePtr = &function.arguments[i];
        i++;
      }
    }




    /// TODO: partially merge with visitVariableDecl
    void visitGlobalVariableDecl(VariableDeclaration *varDecl, IRGenFlags flags) {
      builder.setInsertionBasicBlock(*globalVarInitValueHoldingBB);
      accept(varDecl->initExpression.get(), flags);
      module.globalVariablesInitValues.push_back(globalVarInitValueHoldingBB->instructions.back());
      IRValueVar *initVal = &module.globalVariablesInitValues.back();
      globalVarInitValueHoldingBB->instructions.pop_back();

      IRGlobalVar *valVar = (IRGlobalVar*)varDecl->irVariablePtr;
      valVar->initValue = initVal;
    }


    IRValueVar* visitVariableDecl(VariableDeclaration *varDecl, IRGenFlags flags) override {
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

      IRValueVar *initVal = accept(varDecl->initExpression.get(), flags);
      builder.Instruction(IRStore(valVar, initVal));
      varDecl->irVariablePtr = valVar;

      return valVar;
    }


    IRValueVar *visitFunctionDecl(FunctionDeclaration *funcDecl, IRGenFlags flags) override {
      IRFunction *function = funcDecl->irFunction;
      // insert now into entry bb of the function
      builder.setInsertionBasicBlock(*function->basicBlocks.begin());

      if (funcDecl->body) {
        accept(funcDecl->body.get(), flags);
      }
      return nullptr;
    }


    IRValueVar* visitFunctionParamDeclaration(FunctionParamDeclaration *funcParam, IRGenFlags flags) override {
      IRValueVar *valVar = nullptr;
      IRFunctionArgument &arg = builder.FunctionArgument(funcParam->name);
      arg.astFunctionParamDeclaration = funcParam;
      valVar = (IRValueVar *) &arg;
      arg.name = funcParam->name;
      // this needs to be updated again later genFunctionDefinition(...)
      funcParam->irVariablePtr = valVar;

      if (auto type = dynamic_cast<BuildInType *>(funcParam->type.get())) {
        arg.type = IRTypePointer(langTypeToIRType(type));
        //arg.type = langTypeToIRType(type);
      }
      else {
        error("currently only buildIn types are supported (will ignore var decl)", funcParam->location);
      }

      // handle default expression
      if (funcParam->defaultExpression) {
        // safe current basic block
        IRBasicBlock &curBB = builder.getInsertionBasicBlock();

        // insert args default value expression into global dummy basic block
        builder.setInsertionBasicBlock(*globalVarInitValueHoldingBB);  // note: this also resets the current function
        accept(funcParam->defaultExpression.get(), flags);
        module.globalVariablesInitValues.push_back(globalVarInitValueHoldingBB->instructions.back());
        IRValueVar *initVal = &module.globalVariablesInitValues.back();
        globalVarInitValueHoldingBB->instructions.pop_back();

        // restore current function and BB
        builder.setInsertionBasicBlock(curBB);

        arg.initValue = initVal;
      }
      return valVar;
    }


    IRValueVar* visitClassDecl(ClassDeclaration *classDecl, IRGenFlags flags) override {
      for (auto &e : classDecl->variableDeclarations) {
        accept(e.get(), flags);
      }
      for (auto &e : classDecl->functionDeclarations) {
        accept(&e, flags);
      }
    }



    /// ********************************************************************
    /// Statements

    IRValueVar* visitCompoundStatement(CompoundStatement *st, IRGenFlags flags) override {
      for (auto &child : st->statements) {
        accept(child.get(), flags);
      }
      return nullptr;
    }

    IRValueVar* visitVariableAssignStatement(VariableAssignStatement *st, IRGenFlags flags) override {
      auto type = st->variableExpression->resultType.get();
      auto variablePtr = accept(st->variableExpression.get(), flags.withReturnPointerValue(true)); // @todo this needs to be a pointer, pass returnPointer argument to accept
      auto value = accept(st->valueExpression.get(), flags);

      // if is class type -> copy
      if (auto classType = dynamic_cast<ClassType*>(st->variableExpression->resultType.get())) {
        throw IRGenException("class type currently can't be assigned", st->location);
      }
      // buildIn type
      if (getBuildInTypeFor(type, st->location)->type == BuildIn_str) {
        throw IRGenException("str type currently can't be assigned", st->location);
      }
      if (!holds_alternative<IRTypePointer>(((IRValue*)variablePtr)->type)) {
        throw IRGenInternalException("ir: can't store value to a non pointer value", st->location);
      }

      builder.Instruction(IRStore(variablePtr, value));
      return nullptr;
    }

    IRValueVar* visitReturnStatement(ReturnStatement *st, IRGenFlags flags) override {
      IRValueVar *returnValue = nullptr;
      if (st->expression) {
        returnValue = accept(st->expression->get(), flags);
      }
      return &(IRValueVar&) builder.Instruction(IRReturn(returnValue));
    }

    IRValueVar* visitIfStatement(IfStatement *st, IRGenFlags flags) override {
      AstCodePrinter astPrinter;

      builder.Instruction(IRValueComment("condition for if  \t\t" + astPrinter.getAstAsCode(*st->condition, true)));
      IRValueVar *condition = accept(st->condition.get(), flags);
      IRConditionalJump &condJump = builder.Instruction(IRConditionalJump(condition)); // set jump bbs later

      IRBasicBlock *bbThen = &builder.BasicBlock("ifThen");
      IRBasicBlock *bbEndOfThen; // bb when execution of then is finished, this can but does not have to be bbThen
      condJump.jumpToWhenTrueBB = bbThen;
      accept(st->ifBody.get(), flags);
      bbEndOfThen = &builder.getInsertionBasicBlock();

      IRBasicBlock *bbElse;
      IRBasicBlock *bbEndOfElse; // bb when execution of else is finished, this can but does not have to be bbElse
      if (st->elseBody) {
        bbElse = &builder.BasicBlock("ifElse");
        condJump.jumpToWhenFalseBB = bbElse;
        accept(st->elseBody.get(), flags);
        bbEndOfElse = &builder.getInsertionBasicBlock();
      }

      // jump to this after then or else block
      IRBasicBlock *bbMerge = &builder.BasicBlock("ifMerge");
      // connect bbs when there is an else
      if (st->elseBody) {
        builder.setInsertionBasicBlock(*bbEndOfElse);
        //  builder.Instruction(IRValueComment("end of ifElse from  \t\t" + astPrinter.getAstAsCode(*st->condition, true)));
        builder.Instruction(IRJump(bbMerge)); // jump to merge bb at end of else bb
      }
      // when we have no else
      else {
        condJump.jumpToWhenFalseBB = bbMerge; // on cond is false jump directly to merge bb
      }
      // connect then bb
      builder.setInsertionBasicBlock(*bbEndOfThen);
      // builder.Instruction(IRValueComment("end of ifThen from  \t\t" + astPrinter.getAstAsCode(*st->condition, true)));
      builder.Instruction(IRJump(bbMerge)); // jump to merge bb at end of then bb

      // all instructions after this if are written into the merge bb
      builder.setInsertionBasicBlock(*bbMerge);
      return nullptr;
    }

    IRValueVar* visitWhileStatement(WhileStatement *st, IRGenFlags flags) override {
      accept(st->condition.get(), flags);
      accept(st->body.get(), flags);
      return nullptr;
    }



    /// ********************************************************************
    /// Expressions
    IRValueVar *visitBoolExpression(BoolExpression *ex, IRGenFlags flags) override {
      auto &inst = builder.Instruction(IRConstBoolean());
      inst.value = ex->value;
      return (IRValueVar*)&inst;
    }

    IRValueVar* visitNumberIntExpression(NumberIntExpression *ex, IRGenFlags flags) override {
      auto &inst = builder.Instruction(IRConstNumberI32());
      inst.value = ex->value;
      return (IRValueVar*)&inst;
    }

    IRValueVar* visitNumberFloatExpression(NumberFloatExpression *ex, IRGenFlags flags) override {
      auto &inst = builder.Instruction(IRConstNumberF32());
      inst.value = ex->value;
      return (IRValueVar*)&inst;
    }

    IRValueVar* visitStringExpression(StringExpression *ex, IRGenFlags flags) override {
      throw IRGenInternalException("str currently not supported", ex->location);
    }


    IRValueVar* visitVariableExpression(VariableExpression *ex, IRGenFlags flags) override {
      if (ex->variableDeclaration->irVariablePtr == nullptr) {
        error("no variable allocation generated, can't load variable", ex);
        return nullptr;
      }
      if (flags.returnPointerValue) {
        return ex->variableDeclaration->irVariablePtr;
      }
      else {
        return &(IRValueVar&)builder.Instruction(IRLoad(ex->variableDeclaration->irVariablePtr));
      }
    }

    IRValueVar* visitMemberVariableExpression(MemberVariableExpression *ex, IRGenFlags flags) override {
      accept(ex->parent.get(), flags);
      return nullptr;
    }


    IRValueVar* visitCallExpression(CallExpression *ex, IRGenFlags flags) override {
      // all args that are not found are currently set to nullptr and will be set to the default value in a separate ir pass
      vector<IRValueVar*> args(ex->functionDeclaration->irFunction->arguments.size(), nullptr);
      // match function and call args
      for (int i = 0; i < args.size(); i++) {
        auto &irFunctionArgDef = ex->functionDeclaration->irFunction->getArgument(i);
        auto *argValue = accept(&ex->argumentsNonNamed[i], flags);
        auto argIt = ranges::find_if(ex->argumentsNonNamed, [&](CallExpressionArgument &astCallArg) {
          return astCallArg.argumentDeclaration == irFunctionArgDef.astFunctionParamDeclaration;
        });
        if (argIt == ex->argumentsNonNamed.end()) {
          throw IRGenInternalException("cant't find argument in the function declaration", ex->location);
        }
        long argIndex = argIt - ex->argumentsNonNamed.begin();
        args[argIndex] = argValue;
      }
      for (auto &a : ex->argumentsNonNamed) {
      }
      auto &call = builder.Instruction(IRCall(ex->functionDeclaration->irFunction, args));
      return &(IRValueVar&)call;
    }

    IRValueVar* visitMemberCallExpression(MemberCallExpression *ex, IRGenFlags flags) override {
      accept(ex->parent.get(), flags);
      return visitCallExpression(ex, flags);
    }

    IRValueVar* visitCallExpressionArgument(CallExpressionArgument *ex, IRGenFlags flags) override {
      return accept(ex->expression.get(), flags);
    }


    IRValueVar* visitUnaryExpression(UnaryExpression *ex, IRGenFlags flags) override {
      IRValueVar *toNegate = accept(ex->innerExpression.get(), flags);
      return &(IRValueVar&) builder.Instruction(IRLogicalNot(toNegate));
    }

    IRValueVar* visitBinaryExpression(BinaryExpression *ex, IRGenFlags flags) override {
      auto resultType = dynamic_cast<BuildInType*>(ex->resultType.get());
      auto operandType = dynamic_cast<BuildInType*>(ex->lhs->resultType.get());
      if (!resultType || !operandType) {
        throw IRGenException("only buildIn types are currently supported", ex->location);
      }

      auto lVal = accept(ex->lhs.get(), flags);
      auto rVal = accept(ex->rhs.get(), flags);

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
      return nullptr;
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



