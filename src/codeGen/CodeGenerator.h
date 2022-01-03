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
#include <set>
#include <utility>
#include "BinOperationTypes.h"
#include "exceptions.h"

using namespace std;
using namespace llvm;

/**
 * generate llvm ir from decorated ast.
 */
class CodeGenerator {
  public:
    CodeGenerator(string name):
      builder(context),
      module(name, context),
      dataLayout(&module)
    {
    }

    Module &getModule() {
      return module;
    }


    void generateCode(RootDeclarations &root) {
      genTypes();

      // gen globals
      for (auto &global : root.variableDeclarations) {
        genGlobal(global.get());
      }

      // gen function declarations (no body)
      for (auto &function : root.functionDeclarations) {
        genFunctionDeclaration(&function);
      }


      // gen class types
      for (auto &classDecl : root.classDeclarations) {
        classDecl->llvmStructType = llvm::StructType::create(context);
        classDecl->llvmStructType ->setName("class_" + classDecl->name);
      }
      // assign class types members
      for (auto &classDecl : root.classDeclarations) {
        genClassDeclTypeMembers(classDecl.get());
      }
      // save class sizes
      for (auto &classDecl : root.classDeclarations) {
        classDecl->llvmStructSizeBytes = dataLayout.getTypeAllocSize(classDecl->llvmStructType);
        cout << "-- class '"<< classDecl->name <<"' size: " << classDecl->llvmStructSizeBytes << " bytes" << endl;
      }


      // gen function bodies
      for (auto &function : root.functionDeclarations) {
        if (!function.isExtern) {
          genFunctionBody(&function);
        }
      }
      // class function bodies
      for (auto &classDecl : root.classDeclarations) {
        genClassDeclMemberFunctions(classDecl.get());
      }


      /*
      // main function
      Constant* c = module.getOrInsertFunction(
          "boolTest",
          /*ret type*-/ llvm::Type::getInt1Ty(context),
          llvm::Type::getInt32Ty(context));

      auto* testFunc = cast<Function>(c);
      testFunc->setCallingConv(CallingConv::C);
      auto param1 = testFunc->arg_begin();
      param1->setName("param1");

      // Create a new basic block to start insertion into.
      BasicBlock *BB = BasicBlock::Create(context, "entry", testFunc);
      builder.SetInsertPoint(BB);
      auto val = ConstantInt::get(context, APInt(32, 10, true));
      // auto val2 = ConstantInt::get(context, APInt(32, 11, true));
      auto eq = builder.CreateICmpEQ(val, param1, "comp");
      builder.CreateRet(eq);
      builder.ClearInsertionPoint();

      verifyFunction(*testFunc);
      */

      cout << endl;
      verifyModule(module, &errs());
    }


    /**
     * Create buildIn types like str
     */
    void genTypes() {
      stringType = llvm::StructType::create(context);
      stringType->setName("buildin_str");
      stringType->setBody(
          builder.getInt8PtrTy(), // buffer
          builder.getInt64Ty()    // length
      );
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
      // @todo no dyn_cast
      auto constant = dyn_cast<Constant>(genConstValueExpression(ex));
      auto global = dyn_cast<GlobalVariable>(
          // @todo problem when two globals with same name
          module.getOrInsertGlobal(var->name, getLLvmTypeFor(var->type.get(), var->location))
      );
      global->setInitializer(constant);
      var->llvmVariable = global;
    }


    /**
     * Set type members for class decl, that contains its member variables.
     * When this class uses other class types as members, these will be generated recursively.
     * The llvmType has to be already created for this classDecl before.
     */
    void genClassDeclTypeMembers(ClassDeclaration *classDecl) {
      // loops like 'ClassA has member with type ClassB  and ClassB has member with type ClassA' are not allowed
      if (!checkClassDecl_member_loop(classDecl)) {
        throw CodeGenException("loop in class declaration", classDecl->location);
      }

      // member variables
      vector<Type*> memberVarTypes;
      int i = 0;
      for (auto &memberVar : classDecl->variableDeclarations) {
        Type* llvmMemberType = nullptr;
        auto type = memberVar->type.get();
        // member is class itself (value, not reference)
        if (auto memberClassType = dynamic_cast<ClassType*>(type)) {
          auto memberClassDecl = memberClassType->classDeclaration;
          llvmMemberType = memberClassDecl->llvmStructType;
        }
        // member buildIn type
        else {
          llvmMemberType = getLLvmTypeFor(memberVar->type.get(), memberVar->location);
        }
        if (llvmMemberType) {
          memberVarTypes.push_back(llvmMemberType);
          memberVar->memberIndex = i;
          i++;
        }
      }
      classDecl->llvmStructType->setBody(memberVarTypes);

      // member functions
      for (auto &memberFunc : classDecl->functionDeclarations) {
        genFunctionDeclaration(&memberFunc);
      }

      cout << "-- class type: " << streamInString([&](llvm::raw_ostream &s) {
        classDecl->llvmStructType->print(s);
      }) << endl;
    }

    /**
     * Check if classDecl is contained in one of its members (direct and indirect).
     * If so print error.
     * @return true if no error
     */
    bool checkClassDecl_member_loop(ClassDeclaration *classDecl, list<ClassDeclaration*> parentClassDecls = {}) {
      bool ok = true;
      for (auto &memberVar : classDecl->variableDeclarations) {
        // member is class itself (value, not reference)
        if (auto memberClassType = dynamic_cast<ClassType *>(memberVar->type.get())) {
          auto memberClassDecl = memberClassType->classDeclaration;
          // loops like 'ClassA has member with type ClassB  and ClassB has member with type ClassA' are not allowed
          if (find(parentClassDecls.begin(), parentClassDecls.end(), memberClassDecl) != parentClassDecls.end()) {
            auto msg = printError("code gen", "loop in class '"+classDecl->name+"' declaration: member '"+memberVar->name+"' has type of class that contains '"+ classDecl->name +"' indirectly",
                memberVar->location);
            for (auto it = parentClassDecls.begin(); it != parentClassDecls.end(); it++ ) {
              msg.printMessage("previously referenced from this class", (*it)->location);
            }
            return false;
          }
          // check members members
          else {
            list<ClassDeclaration*> parentClassDeclsOwn = parentClassDecls;
            parentClassDeclsOwn.push_back(memberClassDecl);
            ok &= checkClassDecl_member_loop(memberClassDecl, move(parentClassDeclsOwn));
          }
        }
      }
      return ok;
    }

    /**
     * Gen bodies of class member functions.
     */
    void genClassDeclMemberFunctions(ClassDeclaration *classDecl) {
      for (auto &f : classDecl->functionDeclarations) {
        genFunctionBody(&f);
      }
    }


    void genFunctionDeclaration(FunctionDeclaration *funcDecl, bool isMainFunction= false) {
      auto returnType = getLLvmTypeFor(funcDecl->returnType.get(), funcDecl->location);

      // make function arguments
      std::vector<llvm::Type*> argTypes;
      // if is member function
      if (funcDecl->isMemberFunction()) {
        argTypes.push_back(PointerType::get(funcDecl->parentClass->llvmStructType, 0));
      }
      // functions args
      for (auto &arg : funcDecl->arguments) {
        auto argType = getLLvmTypeFor(arg.type.get(), arg.location);
        argTypes.push_back(argType);
      }

      // create func
      FunctionType *funcType = FunctionType::get(
          returnType,
          argTypes,
          false);

      auto name = funcDecl->isMemberFunction() ? funcDecl->parentClass->name + "_" + funcDecl->name : funcDecl->name;
      Function *func = Function::Create(
          funcType,
          GlobalValue::LinkageTypes::ExternalLinkage,
          name,
          &module);

      // assign names to function args
      auto funcArgsIter = func->args().begin();
      // first arg is 'this' if is member function
      if (funcDecl->isMemberFunction()) {
        funcArgsIter->setName("this");
        funcArgsIter++;
      }
      for (auto &funcDeclArg : funcDecl->arguments) {
        funcArgsIter->setName(funcDeclArg.name);
        funcArgsIter++;
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

      // arguments
      auto funcArgsIter = func->args().begin();
      // create pointer to this argument if its a member function
      if (funcDecl->isMemberFunction()) {
        auto thisVar = funcDecl->parentClass->thisVarDecl.get();
        // store value to ptr
        //auto varPtr = builder.CreateAlloca(PointerType::get(funcDecl->parentClass->llvmStructType, 0), nullptr, "thisPtr");
        //builder.CreateStore(funcArgsIter, varPtr);
        thisVar->llvmVariable = funcArgsIter;
        funcArgsIter++;
      }

      // create pointer to function arguments
      auto funcDeclArgsIter = funcDecl->arguments.begin();
      for (;funcArgsIter != func->args().end(); funcArgsIter++) {
        // store value to ptr
        auto varPtr = builder.CreateAlloca(getLLvmTypeFor(funcDeclArgsIter->type.get(), funcDecl->location), nullptr, funcDeclArgsIter->name + "Ptr");
        builder.CreateStore(funcArgsIter, varPtr);
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
      // while
      else if (auto* st = dynamic_cast<WhileStatement*>(statement)) {
        return genWhileStatement(st);
      }
      // expression
      else if (auto* st = dynamic_cast<Expression*>(statement)) {
        genExpression(st);
        return false;
      }

      // abort compilation
      throw CodeGenException("unsupported statement", statement->location);
      return false;
    }


    void genVariableDeclaration(VariableDeclaration *st) {
      auto init = genExpression(st->initExpression.get());

      // if is class type
      if (auto classType = dynamic_cast<ClassType*>(st->type.get())) {
        auto llvmType = classType->classDeclaration->llvmStructType;
        auto var = builder.CreateAlloca(llvmType, nullptr, st->name);
        // copy init into var
        // @todo avoid copy when init is an R-value, for example a constructor call
        builder.CreateMemCpy(var, 0, init, 0, classType->classDeclaration->llvmStructSizeBytes);
        st->llvmVariable = var;
        return;
      }


      // is build type
      auto typeBuildIn = getBuildInTypeFor(st->type.get(), st->location);

      cout << "- Var Decl: " << st->name << endl;
      cout << "-- init type: " << streamInString([&](llvm::raw_ostream &s) {
        init->getType()->print(llvm::outs());
      }) << endl;

      llvm::Type* type;
      Value* varPtr;

      // str
      if (typeBuildIn->type == BuildIn_str) {
        type = init->getType();
        varPtr = builder.CreateAlloca(type, nullptr, st->name);
      }
      // buildIn
      else {
        type = getLLvmTypeFor(st->type.get(), st->location);
        varPtr = builder.CreateAlloca(type, nullptr, st->name);
      }

      // if init is str
      if (typeBuildIn->type == BuildIn_str) {
        // copy static str into variable
        auto strFirstElement = builder.CreateConstGEP2_64(init, 0, 0, "strFirstElement");
        auto varStrPtr = builder.CreateLoad(varPtr);
        uint64_t strSize = init->getType()->getPointerElementType()->getArrayNumElements();
        builder.CreateMemCpy(varStrPtr, 0, strFirstElement, 0, strSize);
      }
      else {
        builder.CreateStore(init, varPtr);
      }
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
      auto type = statement->variableExpression->resultType.get();
      auto variablePtr = genExpression(statement->variableExpression.get(), true);
      auto value = genExpression(statement->valueExpression.get());

      // if is class type -> copy
      if (auto classType = dynamic_cast<ClassType*>(statement->variableExpression->resultType.get())) {
        auto llvmType = classType->classDeclaration->llvmStructType;
        // copy value into var, @todo: instead of 0 in this call use MaybeAlign()
        builder.CreateMemCpy(variablePtr, 0, value, 0, classType->classDeclaration->llvmStructSizeBytes);
        return;
      }
      // buildIn type
      if (getBuildInTypeFor(type, statement->location)->type == BuildIn_str) {
        throw CodeGenException("str type currently can't be assigned", statement->location);
      }
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
     * @return true when if and else body have return
     */
    bool genWhileStatement(WhileStatement *statement) {
      // get the current function
      // the if belongs to the function
      Function *func = builder.GetInsertBlock()->getParent();

      // add loopCheck, loopBody, loopMerge block
      BasicBlock *checkBlock = BasicBlock::Create(context, "loopCheck", func);
      BasicBlock *bodyBlock = BasicBlock::Create(context, "loopBody", func);
      BasicBlock *mergeBlock = BasicBlock::Create(context, "loopMerge", func);

      // jump to loop check
      builder.CreateBr(checkBlock);

      // create code for loop check
      builder.SetInsertPoint(checkBlock);
      auto conditionVal = genExpression(statement->condition.get());
      builder.CreateCondBr(conditionVal, bodyBlock, mergeBlock);

      // create code for loop body
      builder.SetInsertPoint(bodyBlock);
      bool hasReturn = genCompoundStatement(statement->body.get()); // could have return
      if (!hasReturn) {
        builder.CreateBr(checkBlock);
      }

      // merge block
      builder.SetInsertPoint(mergeBlock);
      // even if body contains return, checkBlock could directly jump to mergeBlock
      return false;
    }





    /**
     * ** Expressions **********************************************
     */

     /**
      * @param returnPointer if true a pointer to the value is returned,
      * otherwise a load is performed and the value itself returned.
      */
    Value *genExpression(Expression *expression, bool returnPointer = false) {
      // constant
      if (auto* ex = dynamic_cast<ConstValueExpression*>(expression)) {
        return genConstValueExpression(ex);
      }
      // call
      else if (auto* ex = dynamic_cast<CallExpression*>(expression)) {
        return genCallExpression(ex);
      }
      // unary
      else if (auto* ex = dynamic_cast<UnaryExpression*>(expression)) {
        return genUnaryExpression(ex);
      }
      // binary
      else if (auto* ex = dynamic_cast<BinaryExpression*>(expression)) {
        return genBinaryExpression(ex);
      }
      // variable
      else if (auto* ex = dynamic_cast<VariableExpression*>(expression)) {
        return genVariableExpression(ex, returnPointer);
      }

      printError("code gen", "unsupported expression", expression->location);
      throw CodeGenException("unsupported expression", expression->location);
    }



    /**
     * For primitive types the direct value will be returned (example i32).
     * For class types a pointer to class struct will be returned (*struct).
     *
     * @param returnPointer if true a pointer to the value is returned,
     * otherwise a load is performed and the value itself returned.
     */
    Value *genVariableExpression(VariableExpression *expression, bool returnPointer) {
      // if is a member variable
      if (auto memberVar = dynamic_cast<MemberVariableExpression*>(expression)) {
        auto parentExprValue = genExpression(memberVar->parent.get(), false);
        auto parentClass = memberVar->variableDeclaration->parentClass;
        auto memberIndex = memberVar->variableDeclaration->memberIndex;
        // get member element from parent
        auto valPtr = builder.CreateConstGEP2_32(parentClass->llvmStructType, parentExprValue, 0, memberIndex);
        // is class -> return pointer
        if (expression->resultType->isClassType() || returnPointer) {
          return valPtr;
        }
        // value otherwise
        else {
          return builder.CreateLoad(valPtr, expression->variableDeclaration->name + "Value");
        }
      }
      // not a member
      else {
        // is class -> return pointer
        if (expression->resultType->isClassType() || returnPointer) {
          return expression->variableDeclaration->llvmVariable;
        }
        // value otherwise
        else {
          if (!expression->variableDeclaration->llvmVariable) {
            throw CodeGenException("llvmVariable of variableDeclaration of expression is null", expression->location);
          }
          return builder.CreateLoad(expression->variableDeclaration->llvmVariable, expression->variableDeclaration->name + "Value");
        }
      }
    }


    Value *genUnaryExpression(UnaryExpression *expression) {
      auto inner = genExpression(expression->innerExpression.get());
      switch (expression->operation) {
        case Expr_Unary_Op_LOGIC_NOT:
          return builder.CreateNot(inner, "tmpNot");
      }
      throw CodeGenException("unsupported unary expression", expression->location);
    }


    Value *genBinaryExpression(BinaryExpression *expression) {
      auto resultType = dynamic_cast<BuildInType*>(expression->resultType.get());
      auto operandType = dynamic_cast<BuildInType*>(expression->lhs->resultType.get());
      if (!resultType || !operandType) {
        throw CodeGenException("only buildIn types are currently supported", expression->location);
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
        else {
          throw CodeGenException("binary compare expression works only with numerical types", expression->location);
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
        case EXPR_OP_LOGIC_AND:
          return  builder.CreateAnd(lhs, rhs, "tmpAnd");
        case EXPR_OP_LOGIC_OR:
          return  builder.CreateOr(lhs, rhs, "tmpOr");
      }

      printError("code gen", "unprocessable binary expression", expression->location);
      throw CodeGenException("unsupported statement", expression->location);
      return nullptr;
    }


    /**
     * Creates call to global function or member function.
     */
    Value *genCallExpression(CallExpression *expression) {
      // call to constructor or class member
      if (expression->functionDeclaration->isMemberFunction()) {
        if (expression->functionDeclaration->isConstructor) {
          return genConstructorCall(expression);
        }
        else if (auto memberCall = dynamic_cast<MemberCallExpression*>(expression)) {
          return genMemberCall(memberCall);
        }
        throw CodeGenException("expression not supported", expression->location);
      }
      // call to global function
      // make args
      vector<Value*> args;
      for (auto &arg: expression->argumentsNonNamed) {
        args.push_back(genExpression(arg.expression.get()));
      }
      auto name = expression->functionDeclaration->returnType->isVoidType() ? "" : "call" + expression->functionDeclaration->name;
      return builder.CreateCall(expression->functionDeclaration->llvmFunction, args, name);
    }

    /**
     * Creates call constructor of a class.
     */
    Value *genConstructorCall(CallExpression *expression) {
      auto functionDecl = expression->functionDeclaration;
      auto parentClassDecl = functionDecl->parentClass;
      return builder.CreateAlloca(parentClassDecl->llvmStructType, nullptr, "construct_object_" + parentClassDecl->name);
    }

    /**
     * Creates call to member function of existing object.
     */
    Value *genMemberCall(MemberCallExpression *expression) {
      auto functionDecl = expression->functionDeclaration;
      auto parentValue = genExpression(expression->parent.get(), true); // return pointer to parent object, no copy
      // args
      vector<Value*> args;
      args.push_back(parentValue); // this arg
      for (auto &arg: expression->argumentsNonNamed) {
        args.push_back(genExpression(arg.expression.get()));
      }
      auto name = expression->functionDeclaration->returnType->isVoidType() ? "" : "call" + expression->functionDeclaration->name;
      return builder.CreateCall(expression->functionDeclaration->llvmFunction, args, name);
    }


    Value *genConstValueExpression(ConstValueExpression *expression) {
      if (auto* ex = dynamic_cast<NumberIntExpression*>(expression)) {
        return genConstIntExpression(ex);
      }
      else if (auto* ex = dynamic_cast<NumberFloatExpression*>(expression)) {
        return genConstFloatExpression(ex);
      }
      else if (auto* ex = dynamic_cast<BoolExpression*>(expression)) {
        return genConstBoolExpression(ex);
      }
      else if (auto* ex = dynamic_cast<StringExpression*>(expression)) {
        return genConstStrExpression(ex);
      }

      throw CodeGenException("unknown const value expression", expression->location);
    }

    Constant *genConstIntExpression(NumberIntExpression *intExpr) {
      return ConstantInt::get(context, APInt(32, intExpr->value, true));
    }

    Constant *genConstFloatExpression(NumberFloatExpression *expr) {
      return ConstantFP::get(context, APFloat(expr->value));
    }

    Constant *genConstBoolExpression(BoolExpression *expr) {
      return ConstantInt::get(context, APInt(1, expr->value, false));
    }

    Constant *genConstStrExpression(StringExpression *expr) {
      // @todo same treatment as other constants, will later create instance of str class
      auto charType = Type::getInt8Ty(context);
      auto strType = ArrayType::get(charType, expr->value.size());
      auto strLength = expr->value.size();
      // copy string
      vector<Constant*> strContend(strLength);
      for (int i = 0; i < strLength; i++) {
        // cout << "'" << expr->value[i] << "'" << endl;
        strContend[i] = ConstantInt::get(charType, expr->value[i]);
      }
      auto strConstant = ConstantArray::get(strType, strContend);

      // add global variable
      auto *globalVar = new GlobalVariable(module, strType, true,
                                    GlobalValue::PrivateLinkage, strConstant, ".str");
      globalVar->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

      // load address of str
      auto strAddress = builder.CreateConstInBoundsGEP2_64(globalVar,0, 0, "strAddress");

      // printLLvmIr();
      return globalVar;
    }




    llvm::Type *getLLvmTypeFor(LangType *type, SrcLocationRange location) {
      auto typeBuildIn = getBuildInTypeFor(type, location);
      return getTypeForBuildIn(typeBuildIn->type);
    }

    BuildInType *getBuildInTypeFor(LangType *type, SrcLocationRange location) {
      auto typeBuildIn = dynamic_cast<BuildInType*>(type);
      if (!typeBuildIn) {
        throw CodeGenException("only buildIn types are currently supported", std::move(location));
      }
      return typeBuildIn;
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
        case BuildIn_str:  // @todo str will become not buildIn, but predefined class type
          return llvm::Type::getInt8PtrTy(context);

        default:
          return nullptr;
      }
    }

  private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
    llvm::DataLayout dataLayout;

    llvm::StructType* stringType;
};