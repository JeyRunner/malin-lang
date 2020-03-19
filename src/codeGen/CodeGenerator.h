#pragma once
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <string>
#include "BinOperationTypes.h"

using namespace std;
using namespace llvm;

/**
 * generate llvm ir from decorated ast.
 */
class CodeGenerator {
  public:
    CodeGenerator(string name):
      builder(context),
      module(name, context)
    {
    }

    Module &getModule() {
      return module;
    }

    Function *funcPutChar;
    void generateCode(RootDeclarations &root) {

      // gen globals
      for (auto &global : root.variableDeclarations) {
        genGlobal(global.get());
      }

      // gen function declarations (no body)
      for (auto &function : root.functionDeclarations) {
        genFunctionDeclaration(&function);
      }

      // gen function bodys
      for (auto &function : root.functionDeclarations) {
        if (!function.isExtern) {
          genFunctionBody(&function);
        }
      }


      /*
      // main function
      Constant* c = module.getOrInsertFunction(
          "boolTest",
          /*ret type*-/ llvm::Type::getInt1Ty(context),
          llvm::Type::getInt32Ty(context));

      auto* testFunc = cast<Function>(c);
      testFunc->setCallingConv(CallingConv::C);
      auto param = testFunc->arg_begin();
      param->setName("param");

      // Create a new basic block to start insertion into.
      BasicBlock *BB = BasicBlock::Create(context, "entry", testFunc);
      builder.SetInsertPoint(BB);
      auto val = ConstantInt::get(context, APInt(32, 10, true));
      // auto val2 = ConstantInt::get(context, APInt(32, 11, true));
      auto eq = builder.CreateICmpEQ(val, param, "comp");
      builder.CreateRet(eq);
      builder.ClearInsertionPoint();

      verifyFunction(*testFunc);
      */

      cout << endl;
      verifyModule(module, &errs());
    }


    void printLLvmIr() {
      module.print(outs(), nullptr);
    }


  private:
    void genGlobal(VariableDeclaration *var) {
      auto* ex = dynamic_cast<NumberExpression*>(var->initExpression.get());
      if (!ex) {
        printError("", "globals need to have a constant init expression -> global ignored", var->location);
        return;
      }
      auto constant = genConstValueExpression(ex);
      auto global = dyn_cast<GlobalVariable>(
          module.getOrInsertGlobal(var->name, getLLvmTypeFor(var->type.get()))
      );
      global->setInitializer(constant);
      var->llvmVariable = global;
    }


    void genFunctionDeclaration(FunctionDeclaration *funcDecl, bool isMainFunction= false) {
      auto returnType = getLLvmTypeFor(funcDecl->returnType.get());

      // make function arguments
      std::vector<llvm::Type*> argTypes;
      for (auto &arg : funcDecl->arguments) {
        auto argType = getLLvmTypeFor(arg.type.get());
        argTypes.push_back(argType);
      }

      FunctionType *funcType = FunctionType::get(
          returnType,
          argTypes,
          false);

      auto name = funcDecl->name;
      Function *func = Function::Create(
          funcType,
          GlobalValue::LinkageTypes::ExternalLinkage,
          name,
          &module);

      // assign names to function args
      auto funcDeclArgsIter = funcDecl->arguments.begin();
      for (auto &arg : func->args()) {
        arg.setName(funcDeclArgsIter->name);
        funcDeclArgsIter++;
      }

      if (isMainFunction) {
        func->setCallingConv(CallingConv::C);
      }
      funcDecl->llvmFunction = func;
      verifyFunction(*func, &errs());
    }


    void genFunctionBody(FunctionDeclaration *funcDecl) {
      auto func = funcDecl->llvmFunction;

      // Create a new basic block to start insertion into.
      BasicBlock *BB = BasicBlock::Create(context, "entry", func);
      builder.SetInsertPoint(BB);

      // create pointer to function arguments
      auto funcDeclArgsIter = funcDecl->arguments.begin();
      for (auto &arg : func->args()) {
        // store value to ptr
        auto varPtr = builder.CreateAlloca(getLLvmTypeFor(funcDeclArgsIter->type.get()), nullptr, funcDeclArgsIter->name + "Ptr");
        builder.CreateStore(&arg, varPtr);
        funcDeclArgsIter->llvmVariable = varPtr;
        funcDeclArgsIter++;
      }

      //builder.CreateRet(ConstantInt::get(context, APInt(32, 1, false)));
      // statements
      genCompoundStatement(funcDecl->body.get());

      builder.ClearInsertionPoint();
      verifyFunction(*func, &errs());
    }


    /**
     * @return true if statement is return
     */
    bool genStatement(Statement *statement) {
      // return
      if (auto* st = dynamic_cast<ReturnStatement*>(statement)) {
        if (st->returnType->isVoidType()) {
          builder.CreateRetVoid();
        }
        else {
          auto value = genExpression(st->expression->get());
          builder.CreateRet(value);
        }
        return true;
      }
      // variable declaration
      else if (auto* st = dynamic_cast<VariableDeclaration*>(statement)) {
        genVariableDeclaration(st);
        return false;
      }
      // variable assign
      else if (auto* st = dynamic_cast<VariableAssignStatement*>(statement)) {
        genVariableAssignStatement(st);
        return false;
      }
      // compound
      else if (auto* st = dynamic_cast<CompoundStatement*>(statement)) {
        return genCompoundStatement(st);
      }
      // if
      else if (auto* st = dynamic_cast<IfStatement*>(statement)) {
        return genIfStatement(st);
      }
      // expression
      else if (auto* st = dynamic_cast<Expression*>(statement)) {
        genExpression(st);
        return false;
      }

      printError("", "ignored unsupported statement", statement->location);
      return false;
    }


    void genVariableDeclaration(VariableDeclaration *st) {
      auto init = genExpression(st->initExpression.get());
      auto varPtr = builder.CreateAlloca(getLLvmTypeFor(st->type.get()), nullptr, st->name);
      builder.CreateStore(init, varPtr);
      st->llvmVariable = varPtr;
    }


    /**
     * @return true if one child statement is return
     */
    bool genCompoundStatement(CompoundStatement *statement) {
      for (auto &st : statement->statements) {
        bool isReturn = genStatement(st.get());
        if (isReturn) {
          return true;
        }
      }
      return false;
    }


    void genVariableAssignStatement(VariableAssignStatement *statement) {
      auto variablePtr = statement->variableExpression->variableDeclaration->llvmVariable;
      auto value = genExpression(statement->valueExpression.get());
      builder.CreateStore(value, variablePtr);
    }


    /**
     * @return true when if and else body have return
     */
    bool genIfStatement(IfStatement *statement) {
      auto conditionVal = genExpression(statement->condition.get());
      bool hasElse = !!statement->elseBody;

      // get the current function
      // the if belongs to the function
      Function *func = builder.GetInsertBlock()->getParent();

      // add if-then, if-else, if-merge block
      BasicBlock *thenBlock = BasicBlock::Create(context, "ifThen", func);
      BasicBlock *elseBlock;
      BasicBlock *mergeBlock = BasicBlock::Create(context, "ifMerge", func);

      if (hasElse) {
        elseBlock = BasicBlock::Create(context, "ifElse", func);
      } else {
        elseBlock = mergeBlock;
      }

      // jump depending on condition
      builder.CreateCondBr(conditionVal, thenBlock, elseBlock);

      // create code for then
      bool hasReturn;
      builder.SetInsertPoint(thenBlock);
      bool thenHasReturn = genCompoundStatement(statement->ifBody.get());
      if (!thenHasReturn) {
        builder.CreateBr(mergeBlock);
      }
      hasReturn = thenHasReturn;

      // create code for else
      if (hasElse) {
        builder.SetInsertPoint(elseBlock);
        bool elseHasReturn = genCompoundStatement(statement->elseBody.get());
        if (!elseHasReturn) {
          builder.CreateBr(mergeBlock);
        }
        hasReturn = hasReturn && elseHasReturn;
      }
      else {
        hasReturn = false;
      }

      if (hasReturn) {
        mergeBlock->eraseFromParent();
      }

      builder.SetInsertPoint(mergeBlock);
      return hasReturn;
    }



    /**
     * ** Expressions **********************************************
     */
    Value *genExpression(Expression *expression) {
      // constant
      if (auto* ex = dynamic_cast<ConstValueExpression*>(expression)) {
        return genConstValueExpression(ex);
      }
      // call
      else if (auto* ex = dynamic_cast<CallExpression*>(expression)) {
        return genCallExpression(ex);
      }
      // binary
      else if (auto* ex = dynamic_cast<BinaryExpression*>(expression)) {
        return genBinaryExpression(ex);
      }
      // variable
      else if (auto* ex = dynamic_cast<VariableExpression*>(expression)) {
        return genVariableExpression(ex);
      }

      printWarn("", "ignored expression -> will use -1 as value", expression->location);
      return ConstantInt::get(context, APInt(32, -1, false));
    }


    Value *genVariableExpression(VariableExpression *expression) {
      return builder.CreateLoad(expression->variableDeclaration->llvmVariable, expression->variableDeclaration->name + "Value");
    }



    Value *genBinaryExpression(BinaryExpression *expression) {
      auto resultType = dynamic_cast<BuildInType*>(expression->resultType.get());
      auto operandType = dynamic_cast<BuildInType*>(expression->lhs->resultType.get());
      if (!resultType || !operandType) {
        throw runtime_error("only buildIn types are currently supported");
      }

      auto lhs = genExpression(expression->lhs.get());
      auto rhs = genExpression(expression->rhs.get());


      // when operation is compare
      CompareInfo compare = getBinaryOperationCompare(expression->operation, operandType->type);
      if (compare.isCompare) {
        if (operandType->type == BuildIn_i32) {
          return builder.CreateICmp(compare.pred, lhs, rhs, "tmpICmp");
        }
        else if (operandType->type == BuildIn_f32) {
          return builder.CreateFCmp(compare.pred, lhs, rhs, "tmpFCmp");
        }
      }


      // when other operation
      switch (expression->operation) {
        case Expr_Op_Plus: {
          switch (resultType->type) {
            case BuildIn_i32:
              return builder.CreateAdd(lhs, rhs, "tmpAdd");
            case BuildIn_f32:
              return builder.CreateFAdd(lhs, rhs, "tmpFAdd");
          }
          break;
        }
        case Expr_Op_Minus: {
          switch (resultType->type) {
            case BuildIn_i32:
              return builder.CreateSub(lhs, rhs, "tmpSub");
            case BuildIn_f32:
              return builder.CreateFSub(lhs, rhs, "tmpFSub");
          } break;
        }
        case Expr_Op_Divide: {
          switch (resultType->type) {
            case BuildIn_i32:
              return builder.CreateSDiv(lhs, rhs, "tmpSDiv");
            case BuildIn_f32:
              return builder.CreateFDiv(lhs, rhs, "tmpFDiv");
          } break;
        }
        case Expr_Op_Multiply: {
          switch (resultType->type) {
            case BuildIn_i32:
              return builder.CreateMul(lhs, rhs, "tmpMul");
            case BuildIn_f32:
              return builder.CreateFMul(lhs, rhs, "tmpFMul");
          } break;
        }
      }

      printError("code gen", "unprocessable binary expression", expression->location);
      return nullptr;
    }



    Value *genCallExpression(CallExpression *expression) {
      // make args
      vector<Value*> args;
      for (auto &arg: expression->argumentsNonNamed) {
        args.push_back(genExpression(arg.expression.get()));
      }
      auto name = expression->functionDeclaration->returnType->isVoidType() ? "" : "call" + expression->functionDeclaration->name;
      return builder.CreateCall(expression->functionDeclaration->llvmFunction, args, name);
    }


    Constant *genConstValueExpression(ConstValueExpression *expression) {
      if (auto* ex = dynamic_cast<NumberIntExpression*>(expression)) {
        return genConstIntExpression(ex);
      }
      else if (auto* ex = dynamic_cast<NumberFloatExpression*>(expression)) {
        return genConstFloatExpression(ex);
      }
      else if (auto* ex = dynamic_cast<BoolExpression*>(expression)) {
        return genConstBoolExpression(ex);
      }

      throw runtime_error("unknown expression");
    }

    Constant *genConstIntExpression(NumberIntExpression *intExpr) {
      return ConstantInt::get(context, APInt(32, intExpr->value, true));
    }

    Constant *genConstFloatExpression(NumberFloatExpression *expr) {
      return ConstantFP::get(context, APFloat(expr->value));
    }

    Constant *genConstBoolExpression(BoolExpression *intExpr) {
      return ConstantInt::get(context, APInt(1, intExpr->value, false));
    }





    llvm::Type *getLLvmTypeFor(LangType *type) {
      auto typeBuildIn = dynamic_cast<BuildInType*>(type);
      if (!typeBuildIn) {
        throw runtime_error("only buildIn types are currently supported");
      }
      return getTypeForBuildIn(typeBuildIn->type);
    }

    llvm::Type *getTypeForBuildIn(BUILD_IN_TYPE buildIn) {
      switch (buildIn) {
        case BuildIn_i32:
          return llvm::Type::getInt32Ty(context);
        case BuildIn_f32:
          return llvm::Type::getFloatTy(context);
        case BuildIn_void:
          return llvm::Type::getVoidTy(context);
        case BuildIn_bool:
          return llvm::Type::getInt1Ty(context);

        default:
          return nullptr;
      }
    }

  private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
};