#include <iostream>
#include <experimental/filesystem>
#include <lyra/lyra.hpp>
#include <fstream>
#include <termcolor/termcolor.hpp>
#include "Log.h"
#include "File.hpp"
#include "Lexer.h"
#include "parser/Parser.h"

using namespace std;
using namespace lyra;
namespace fs = std::experimental::filesystem;

// cli args
bool debug = false;
bool showLexerOutput = false;
string srcFile;


/**
 * parse cli arguments
 */
void parseCliArgs(const args arguments) {
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


  // parse args
  auto cli_result = cli.parse(arguments);

  // Check that the arguments where valid:
  if (!cli_result){
    std::cerr << "Error in command line: " << cli_result.errorMessage() << std::endl;
    exit(1);
  }

  if (showHelp){
    std::cout << cli << std::endl;
    exit(0);
  }
}


/**
 * Program entry point
 */
int main(int argc, const char **argv)
{
  cout << "The " << termcolor::underline << termcolor::green <<"malin language" << termcolor::reset << " compiler" << endl << endl;

  // cli args
   parseCliArgs({argc, argv});


  // start
  cout << "- will compile file '" << srcFile << "'" << endl;


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
  cout << "-- file has " << fileContend.size() << " characters" << endl;


  // -------------------------------
  // -- lexing
  cout << termcolor::bold << "- lexing:" << termcolor::reset << endl;
  Lexer lexer(fileContend);

  list<Token> tokenList;
  try {
    tokenList = lexer.getAllTokens();
    cout << "-- lexing done" << endl;
  }
  catch (exception &e) {
    error("Error while lexing", e);
    return -1;
  }

  if (showLexerOutput) {
    for (Token token : tokenList) {
      cout << fs::canonical(filePath).string() << ":" << token.location.start.toString() << ": " /* << endl << "\t\t" */ << token.toString() << endl;
    }
  }


  // -------------------------------
  // -- parsing
  cout << termcolor::bold << "- parsing:" << termcolor::reset << endl;
  Parser parser(move(tokenList));
  try {
    RootDeclarations root = parser.parse();
    root.print();
  }
  catch (ParseException &e) {
    cout << fs::canonical(filePath).string() << ":" << e.token.location.start.toString() << ": "
      << termcolor::bold << termcolor::red << "parse error: " "" << e.text << endl;
  }


  return 0;
}
