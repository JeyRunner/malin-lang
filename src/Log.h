/* 
 * File:   Log.h
 * Author: Joshua Johannson
 *
  */
#pragma once


#include <iostream>
#include <termcolor/termcolor.hpp>
using namespace std;

void error(string msg, exception &e) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ": " << e.what() << endl;
}

void error(string msg) {
  cerr << termcolor::white << "-- " << termcolor::bold << termcolor::red << "[!!]" << termcolor::reset << " " << msg << ""<< endl;
}
