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
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include <string>

using namespace std;
using namespace llvm;

class CodeEmitter {
  public:
    static void emitObjectFile(Module &module) {
      auto targetTriple = sys::getDefaultTargetTriple();
      cout << "-- will compile for '"+ targetTriple +"'" << endl;

      // init all
      InitializeAllTargetInfos();
      InitializeAllTargets();
      InitializeAllTargetMCs();
      InitializeAllAsmParsers();
      InitializeAllAsmPrinters();

      std::string error;
      auto target = TargetRegistry::lookupTarget(targetTriple, error);

      // Print an error and exit if we couldn't find the requested target.
      // This generally occurs if we've forgotten to initialise the
      // TargetRegistry or we have a bogus target triple.
      if (!target){
        errs() << error;
        return;
      }

      auto CPU = "generic";
      auto Features = "";

      TargetOptions opt;
      auto RM = Optional<Reloc::Model>();
      auto targetMachine = target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

      module.setDataLayout(targetMachine->createDataLayout());
      module.setTargetTriple(targetTriple);

      // open file
      auto filename = "output.o";
      std::error_code fileErrorCode;
      raw_fd_ostream dest(filename, fileErrorCode, sys::fs::OpenFlags::F_None);

      if (fileErrorCode) {
        errs() << "-- Could not open file: " << fileErrorCode.message();
        return;
      }

      // add passes
      legacy::PassManager passManager;

      // Promote allocas to registers.
      passManager.add(createPromoteMemoryToRegisterPass());
      // Do simple "peephole" optimizations and bit-twiddling optzns.
      passManager.add(createInstructionCombiningPass());
      // Reassociate expressions.
      passManager.add(createReassociatePass());
      // Eliminate Common SubExpressions.
      passManager.add(createGVNPass());
      // Simplify the control flow graph (deleting unreachable blocks, etc).
      passManager.add(createCFGSimplificationPass());

      if (targetMachine->addPassesToEmitFile(passManager, dest, nullptr, CGFT_ObjectFile)) {
        errs() << "-- TargetMachine can't emit a file of this type";
        return;
      }

      passManager.run(module);
      dest.flush();
      cout << "-- written object file '" << filename << "' " << tc::green << "done" << tc::reset << endl;
    }


    static void emitBitCodeFile(Module &module, string filename) {
      std::error_code errorCode;
      llvm::raw_fd_ostream OS(filename, errorCode, llvm::sys::fs::F_None);
      if (!errorCode) {
        module.print(OS, nullptr);
        OS.flush();
        cout << "-- saved bit code file: " << filename << endl;
      }
      else {
        cout << "-- ERR: while saving bit code file '" << filename << "': " << errorCode.message() << endl;
      }
    }


    /**
     * @deprecated
     */
    static void emitDotFile(Function *func, string filename) {
      std::error_code EC;
      cout << "-- save dot file: " << filename << endl;

      std::error_code errorCode;
      raw_fd_ostream fileStream(filename.c_str(), errorCode, sys::fs::F_Text);

      if (!errorCode) {
        WriteGraph(fileStream, func);
      }
      else
        errs() << "  error opening file for writing!";
      errs() << "\n";
    }
};