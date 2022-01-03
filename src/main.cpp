#include <iostream>
#include <experimental/filesystem>
#include <lyra/lyra.hpp>
#include <fstream>
#include <cstdlib>
#include <termcolor/termcolor.hpp>
#include <ir/builder/exceptions.h>
#include "Log.h"
#include "File.hpp"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "SourceManager.h"
#include "AstVisitor/AstCodePrinter.h"
#include "AstVisitor/AstPrinter.h"
#include "decorator/AstDecorator.h"
#include "ir/gen/IRGenerator.h"
#include "ir/visitor/IRVisitor.h"
#include "ir/printer/IRPrinter.h"

#include "codeGen/CodeGenerator.h"
#include "codeGen/CodeEmitter.h"

#include "util/version.h"
#include "parser/AST_addFunc.h"


using namespace std;
using namespace lyra;
namespace fs = std::experimental::filesystem;

// cli args
bool debug = false;
bool showLexerOutput = false;
bool showParserOutput = false;
bool showDecoratorOutput = false;
bool showAstAsCode = false;
bool saveAstAsCode = false;
bool showLLvmIR = false;
bool saveLLvmIR = false;
bool notWriteObjectFile = false;
bool runCompiled = false;
bool useIR = false;
string viewFunctionLLvmGraph = "";
string srcFile;

void exitWithError();


/**
 * parse cli arguments
 */
void parseCliArgs(const args& arguments) {
  bool showHelp = false;

  // define args
  auto cli = cli_parser();
  cli.add_argument(
      opt(srcFile, "file")
          .name("-f")
          .required()
          .help("source file to compile"));
  cli.add_argument(
      help(showHelp));
  /*
  cli.add_argument(
      opt(debug, "debug")
          .name("-d")
          .help("show debug output"));
          */
  cli.add_argument(
      opt(showLexerOutput)
          .name("--show-lexer-output")
          .help("show the token list output of the lexer"));
  cli.add_argument(
      opt(showParserOutput)
          .name("--show-parser-output")
          .help("shows the parser ast output"));
  cli.add_argument(
      opt(showDecoratorOutput)
          .name("--show-decorator-output")
          .help("shows the ast after identifiers have been linked"));
  cli.add_argument(
      opt(showAstAsCode)
          .name("--show-ast-as-code")
          .help("shows the ast after identifiers have been linked as code"));
  cli.add_argument(
      opt(saveAstAsCode)
          .name("--save-ast-as-code")
          .help("saves the ast after identifiers have been linked as code to the file '<src-file-name>' in the current folder"));
  cli.add_argument(
      opt(showLLvmIR)
          .name("--show-llvm-ir")
          .help("shows the generated llvm ir code"));
  cli.add_argument(
      opt(saveLLvmIR)
          .name("--save-llvm-ir")
          .help("saves the generated llvm ir code to the file '<src-file-name>.ll'"));
  cli.add_argument(
      opt(notWriteObjectFile)
          .name("--not-create-object-file")
          .help("will only create llvm ir"));
  cli.add_argument(
      opt(viewFunctionLLvmGraph, "function name")
          .name("--view-function-graph")
          .help("show graph of the llvm ir for this function"));
  cli.add_argument(
      opt(runCompiled)
          .name("--run")
          .help("runs the compiled program"));
    cli.add_argument(
        opt(useIR)
            .name("--use-ir")
            .help("use experimental intermediate representation for code generation [incomplete]"));


  // parse args
  auto cli_result = cli.parse(arguments);

  // Check that the arguments where valid:
  if (!cli_result){
    std::cerr << "Error in command line: " << cli_result.errorMessage() << std::endl;
    exit(1);
  }

  if (showHelp){
    std::cout << endl << cli << std::endl;
    exit(0);
  }
}


/**
 * Program entry point
 */
int main(int argc, const char **argv)
{
  cout << "The " << termcolor::underline << termcolor::green <<"malin language" << termcolor::reset << " compiler"
       << " v" << getFullVersion() << " (commit: " << getGitCommit() << ")" << endl;

  // cli args
   parseCliArgs({argc, argv});

  // start
  cout << "- will compile file '" << srcFile << "'" << endl << endl;


  // -------------------------------
  // -- read file
  cout << termcolor::bold << "- read file:" << termcolor::reset <<endl;
  string fileContend;
  fs::path filePath(srcFile);
  try {
    fileContend = File::readFile(filePath);
  } catch (runtime_error &e) {
    error("Error while reading file", e);
    exitWithError();
  }
  cout << "-- file has " << fileContend.size() << " characters" << endl << endl;
  sourceManager.setSource(filePath, fileContend);



  // -------------------------------
  // -- lexing
  cout << termcolor::bold << "- lexing:" << termcolor::reset << endl;
  Lexer lexer(fileContend);

  list<Token> tokenList;
  try {
    tokenList = lexer.getAllTokens();
  }
  catch (exception &e) {
    error("Error while lexing", e);
    exitWithError();
  }

  if (showLexerOutput) {
    cout << "-- tokens:" << termcolor::reset << endl;
    for (Token token : tokenList) {
      cout << fs::canonical(filePath).string() << ":" << token.location.start.toString() << ": " /* << endl << "\t\t" */ << token.toString() << endl;
    }
  }
  cout << "-- lexing " << termcolor::green << "done" << termcolor::reset << endl << endl;



  // -------------------------------
  // -- parsing
  cout << termcolor::bold << "- parsing:" << termcolor::reset << endl;
  Parser parser(move(tokenList));
  RootDeclarations root;
  try {
    root = parser.parse();
  }
  catch (ParseException &e) {
    printError(
        "parse",
        e.what(),
        e.token.location);
    exitWithError();
  }

  if (showParserOutput) {
    cout << "-- ast:" << termcolor::reset;
    AstPrinter printer(std::cout);
    printer.printTree(root);
  }
  cout << "-- parsing " << termcolor::green << "done" << termcolor::reset << endl << endl;



  // -------------------------------
  // -- decorate
  cout << termcolor::bold << "- decorate ast:" << termcolor::reset << endl;
  AstDecorator astDecorator;
  bool decoOk = astDecorator.linkNames(root);

  if (showDecoratorOutput) {
    cout << "-- ast:" << termcolor::reset;
    AstPrinter printer(std::cout);
    printer.printTree(root);
  }

  if (!decoOk) {
    exitWithError();
  }
  cout << "-- decorating " << termcolor::green << "done" << termcolor::reset << endl << endl;

  if (showAstAsCode) {
    AstCodePrinter astPrinter;
    cout << endl << "-- Ast as code" << endl;
    cout << astPrinter.getAstAsCode(root);
    cout << endl << endl;
  }
  if (saveAstAsCode) {
    AstCodePrinter astPrinter;
    string code = astPrinter.getAstAsCode(root);
    File::saveFile(filePath.filename().string(), code);
  }


  // -------------------------------
  // -- gen IR
  if (useIR) {
    cout << termcolor::bold << "- IR generation:" << termcolor::reset << endl;
    IRGenerator irGenerator;
    bool irGenOk = true;

    try {
      irGenerator.generate(root, filePath.filename());
    }
    catch(IRGenInternalException &e) {
      cout << endl << "-- ir gen " << termcolor::red << "aborted because of error:" << termcolor::reset << endl;
      printError(
          "[[INTERNAL ERROR]] ir generation",
          e.what(),
          e.location);
      irGenOk = false;
    }
    catch(IRGenException &e) {
      cout << endl << "-- ir gen " << termcolor::red << "aborted because of error:" << termcolor::reset << endl;
      printError(
          "ir generation",
          e.what(),
          e.location);
      irGenOk = false;
    }

    if (irGenOk) {
      cout << "-- IR generation " << termcolor::green << "done" << termcolor::reset << endl << endl;

      cout << endl << "-- IR:" << endl;
      IRPrinter irPrinter(std::cout);
      irPrinter.print(irGenerator.module);
      cout << endl << endl;
    }


    cout << "-- " << termcolor::yellow <<" will not generate code [not implemented yet with IR] " << termcolor::reset << endl << endl;
    exit(2);
  }




  // -------------------------------
  // -- code gen
  cout << termcolor::bold << "- code generation:" << termcolor::reset << endl;
  CodeGenerator codeGen(filePath.filename());
  bool codeGenOk = true;
  try {
    codeGen.generateCode(root);
  } catch(CodeGenException &e) {
    cout << endl << "-- code gen " << termcolor::red << "aborted because of error:" << termcolor::reset << endl;
    printError(
        "code generation",
        e.what(),
        e.location);
    codeGenOk = false;
  }
  if (showLLvmIR) {
    cout << endl << "-- llvm ir:" << termcolor::reset << endl << endl;
    codeGen.printLLvmIr();
    cout << endl << endl;
  }
  if (saveLLvmIR) {
    CodeEmitter::emitBitCodeFile(codeGen.getModule(), filePath.filename().string() + ".ll");
  }

  // codegen successful
  if (codeGenOk) {
    cout << "-- code gen " << termcolor::green << "done" << termcolor::reset << endl << endl;
  }
  // exit if error
  if (!codeGenOk) {
    exitWithError();
  }



  // -------------------------------
  // -- create object file and link
  if (!notWriteObjectFile) {
    CodeEmitter::emitObjectFile(codeGen.getModule());
    // link object file with libmalinGlued and libc
    int linkCode = std::system("clang -o bin.o output.o -l:libmalinCGlue.a -L./std/c -L../lib "); // -lc -dynamic-linker
    cout << "-- linking returned " << linkCode << endl;
    if (linkCode != 0) {
      exitWithError();
    }
  }


  // -------------------------------
  // -- view graph
  if (!viewFunctionLLvmGraph.empty()) {
    auto funcIter = find_if(
        root.functionDeclarations.begin(),
        root.functionDeclarations.end(),
        [](FunctionDeclaration& decl) {
          return decl.name == viewFunctionLLvmGraph;
        });
    if (funcIter == root.functionDeclarations.end()) {
      cout << "ERR:  --view-function-graph "<< viewFunctionLLvmGraph << " function not found" << endl;
      exitWithError();
    }
    auto func = funcIter->llvmFunction;
    func->viewCFG();
  }



  // -------------------------------
  // -- end
  cout << termcolor::bold << termcolor::green << "   Compiled without errors" << termcolor::reset << endl;
  cout << endl;


  // -------------------------------
  // execute program
  if (runCompiled) {
    cout << termcolor::bold << "- executing compiled program:" << termcolor::reset << endl;
    int code = std::system("./bin.o");
    cout << "-- program finished with exit code " << code << endl;
  }

  return 0;
}



void exitWithError() {
  cout << endl;
  cout << termcolor::bold << termcolor::red << "   errors while compiling, abort" << termcolor::reset << endl;
  exit(1);
}