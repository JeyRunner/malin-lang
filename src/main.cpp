#include <iostream>
#include <experimental/filesystem>
#include <lyra/lyra.hpp>
#include <fstream>
#include <termcolor/termcolor.hpp>
#include "Log.h"
#include "File.hpp"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "SourceManager.h"
#include "decorator/AstDecorator.h"

using namespace std;
using namespace lyra;
namespace fs = std::experimental::filesystem;

// cli args
bool debug = false;
bool showLexerOutput = false;
bool showParserOutput = false;
bool showDecoratorOutput = false;
string srcFile;


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
  cli.add_argument(
      opt(debug, "debug")
          .name("-d")
          .help("show debug output"));
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
  cout << "The " << termcolor::underline << termcolor::green <<"malin language" << termcolor::reset << " compiler" << endl;

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
    return 1;
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
    return 1;
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
    return 1;
  }

  if (showParserOutput) {
    cout << "-- ast:" << termcolor::reset << endl;
    root.print(1);
  }
  cout << "-- parsing " << termcolor::green << "done" << termcolor::reset << endl << endl;


  // -------------------------------
  // -- decorate
  cout << termcolor::bold << "- decorate ast:" << termcolor::reset << endl;
  AstDecorator astDecorator;
  bool decoOk = astDecorator.linkNames(root);

  if (showDecoratorOutput) {
    cout << "-- ast:" << termcolor::reset << endl;
    root.print(1);
  }

  if (!decoOk) {
    return 1;
  }
  cout << "-- decorating " << termcolor::green << "done" << termcolor::reset << endl << endl;



  // -------------------------------
  // -- end
  cout << termcolor::bold << "- Executable generation not implemented: " << termcolor::yellow <<"-> skipping" << termcolor::reset << endl;
  cout << endl;
  cout << termcolor::bold << termcolor::green << "   Compiled without errors" << termcolor::reset << endl;
  cout << endl;

  return 0;
}
