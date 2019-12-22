#include <iostream>
#include <experimental/filesystem>
#include <lyra/lyra.hpp>
#include <fstream>
#include "File.hpp"

using namespace std;
using namespace lyra;
namespace fs = std::experimental::filesystem;

// cli args
bool debug;
string srcFile;


/**
 * parse cli arguments
 */
void parseCliArgs(const args arguments) {
  bool showHelp = false;

  // define args
  auto cli = cli_parser();
  cli.add_argument(
      help(showHelp));
  cli.add_argument(
      opt(debug, "debug")
          .name("-d")
          .help("show debug output"));
  cli.add_argument(
      opt(srcFile, "file")
          .name("-f")
          .required()
          .help("source file to compile"));


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
  cout << "The malin language compiler" << endl;

  // cli args
   parseCliArgs({argc, argv});


  // start
  cout << "- will compile '" << srcFile << "'" << endl;

  // read file
  string fileContend;
  try {
    fileContend = File::readFile(fs::path(srcFile));
  } catch (runtime_error &e) {
    cerr << e.what() << endl;
    return 1;
  }

  cout << "- read file:" << endl << fileContend << endl;

  return 0;
}
