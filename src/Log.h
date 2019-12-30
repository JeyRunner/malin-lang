/* 
 * File:   Log.h
 * Author: Joshua Johannson
 *
  */
#pragma once


#include <iostream>
#include <termcolor/termcolor.hpp>
#include <utility>
#include "lexer/Lexer.h"
#include "SourceManager.h"
using namespace std;

using namespace std;
namespace fs = std::experimental::filesystem;
namespace tc = termcolor;


void error(string msg, exception &e) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ": " << e.what() << endl;
}

void error(string msg) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ""<< endl;
}


void printMessage(
    const string& title,
    const string& msg,
    SrcLocationRange &location,
    std::ostream& (*formatter)(std::ostream& stream))
{
  string srcLine = sourceManager.getLine(location.start.line);

  // create src location marker
  string srcLocationIndentation;
  for (int i = 0; i < location.start.column - 1; ++i) {
    srcLocationIndentation += " ";
  }
  string srcLocationMarker = srcLocationIndentation + "^";
  bool endAtOtherLine = false;
  if (location.end) {
    int columnEnd;
    if (location.start.line == location.end->line) {
      columnEnd = location.end->column;
    } else {
      columnEnd = (int)srcLine.size() + 1;
      endAtOtherLine = true;
    }
    for (int i = location.start.column+1; i < columnEnd; ++i) {
      srcLocationMarker += "^";
    }
  }

  // append text to marker
  string textAfterMarker;
  if (endAtOtherLine) {
    int linesDiff = location.end->line - location.start.line;
    srcLocationMarker += " °°° ";
    textAfterMarker = "and next " + to_string(linesDiff) + " lines ";
    textAfterMarker += "until " + location.end->toString();
  }

  // print all
  cout << sourceManager.getFilePathString() << ":" << location.start.toString() << ": "
       << tc::bold << formatter << title << tc::reset << endl
       << " | " << endl
       << " | " << srcLine << endl
       << " | " << formatter << srcLocationMarker << tc::grey << textAfterMarker << tc::reset << endl
       << " | " << formatter << srcLocationIndentation << msg << tc::reset << endl << endl;
}


void printError(
    const string& errorType,
    const string& msg,
    SrcLocationRange &location)
{
  printMessage(
      errorType + " error",
      msg,
      location,
      tc::red);
}


void printWarn(
    const string& warnType,
    const string& msg,
    SrcLocationRange &location)
{
  printMessage(
      warnType + " warning",
      msg,
      location,
      tc::yellow);
}

void printNote(
    const string& title,
    const string& msg,
    SrcLocationRange &location)
{
  printMessage(
      "note: " + title,
      msg,
      location,
      tc::white);
}