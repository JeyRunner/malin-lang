/* 
 * File:   Log.h
 * Author: Joshua Johannson
 *
  */
#pragma once


#include <iostream>
#include <termcolor/termcolor.hpp>
#include "lexer/Lexer.h"
#include "SourceManager.h"

using namespace std;
namespace fs = std::experimental::filesystem;
namespace tc = termcolor;


void error(string msg, exception &e) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ": " << e.what() << endl;
}

void error(string msg) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ""<< endl;
}

void printErrorAtSrcLocation(
    string errorType,
    string msg,
    SrcLocationRange &location,
    fs::path &filePath,
    SourceManager sourceManager)
{
  // create src location marker
  string srcLocationIndentation;
  for (int i = 0; i < location.start.column - 1; ++i) {
    srcLocationIndentation += " ";
  }
  string srcLocationMarker = srcLocationIndentation + "^";
  if (location.end && location.start.line == location.end->line) {
    for (int i = location.start.column+1; i < location.end->column; ++i) {
      srcLocationMarker += "^";
    }
  }

  string srcLine = sourceManager.getLine(location.start.line);
  cout << fs::canonical(filePath).string() << ":" << location.start.toString() << ": "
       << tc::bold << tc::red << errorType << " error" << tc::reset << endl
       << endl
       << srcLine << endl
       << tc::red << srcLocationMarker << endl
       << srcLocationIndentation << msg << tc::reset << endl << endl;
}