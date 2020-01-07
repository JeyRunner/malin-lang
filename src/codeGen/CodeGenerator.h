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

      // add putchar
      /*
      funcPutChar = cast<Function>(
          module.getOrInsertFunction(
              "putChar",
              Type::getVoidTy(context),
              Type::getInt32Ty(context)));
              */


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
      auto returnType = llvm::Type::getInt32Ty(context);
      /*

      vector<llvm::Type*> argTypes();

      FunctionType *funcType = FunctionType::get(returnType, false);
      Function *func = Function::Create(funcType, GlobalValue::LinkageTypes::InternalLinkage,"main", &module);
      *-/

      Constant* c = module.getOrInsertFunction("main",
          /*ret type*-/                          llvm::Type::getInt32Ty(context)
          /*varargs terminated with null*-/    );

      auto* main = cast<Function>(c);
      main->setCallingConv(CallingConv::C);

      // Create a new basic block to start insertion into.
      BasicBlock *BB = BasicBlock::Create(context, "entry", main);
      builder.SetInsertPoint(BB);
      builder.CreateRet(ConstantInt::get(context, APInt(32, 2, true)));

      verifyFunction(*main);
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
      auto constant = genConstNumberExpression(ex);
      var->llvmVariable = constant;

      module.getOrInsertGlobal(var->name, getLLvmTypeFor(var->type.get()));
      module.getNamedGlobal(var->name)->setInitializer(constant);
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
        funcDeclArgsIter->llvmVariable = &arg;
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
          builder.CreateRet(genExpression(st->expression->get()));
        }
        return true;
      }
      // variable declaration
      else if (auto* st = dynamic_cast<VariableDeclaration*>(statement)) {
        st->llvmVariable = genExpression(st->initExpression.get());
        return false;
      }
      // compound
      else if (auto* st = dynamic_cast<CompoundStatement*>(statement)) {
        return genCompoundStatement(st);
      }
      // expression
      else if (auto* st = dynamic_cast<Expression*>(statement)) {
        genExpression(st);
        return false;
      }

      return false;
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



    /**
     * ** Expressions **********************************************
     */
    Value *genExpression(Expression *expression) {
      // constant
      if (auto* ex = dynamic_cast<NumberExpression*>(expression)) {
        return genConstNumberExpression(ex);
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
      return expression->variableDeclaration->llvmVariable;
    }


    Value *genBinaryExpression(BinaryExpression *expression) {
      auto typeBuildIn = dynamic_cast<BuildInType*>(expression->resultType.get());
      if (!typeBuildIn) {
        throw runtime_error("only buildIn types are currently supported");
      }

      auto lhs = genExpression(expression->lhs.get());
      auto rhs = genExpression(expression->rhs.get());

      switch (expression->operation) {
        case EXPR_OP_GREATER_THEN:
          return nullptr;
        case EXPR_OP_SMALLER_THEN:
          return nullptr;
        case Expr_Op_Plus: {
          switch (typeBuildIn->type) {
            case BuildIn_i32:
              return builder.CreateAdd(lhs, rhs, "tmpAdd");
            case BuildIn_f32:
              return builder.CreateFAdd(lhs, rhs, "tmpAdd");
          }
        }
        case Expr_Op_Minus:{
          switch (typeBuildIn->type) {
            case BuildIn_i32:
              return builder.CreateSub(lhs, rhs, "tmpAdd");
            case BuildIn_f32:
              return builder.CreateFSub(lhs, rhs, "tmpAdd");
          }
        }
        case Expr_Op_Divide:
          return builder.CreateFDiv(lhs, rhs, "tmpDiv");
        case Expr_Op_Multiply:
          return builder.CreateMul(lhs, rhs, "tmpMul");
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


    Constant *genConstNumberExpression(NumberExpression *expression) {
      if (auto* ex = dynamic_cast<NumberIntExpression*>(expression)) {
        return genConstIntExpression(ex);
      }
      else if (auto* ex = dynamic_cast<NumberFloatExpression*>(expression)) {
        return genConstFloatExpression(ex);
      }

      throw runtime_error("unknown expression");
    }

    Constant *genConstIntExpression(NumberIntExpression *intExpr) {
      return ConstantInt::get(context, APInt(32, intExpr->value, true));
    }

    Constant *genConstFloatExpression(NumberFloatExpression *expr) {
      return ConstantFP::get(context, APFloat(expr->value));
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