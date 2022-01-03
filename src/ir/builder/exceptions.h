#pragma once

#include<iostream>
#include <utility>
#include <Log.h>
using namespace std;


/**
 * Exception of the IRGenerator.
 */
class IRGenException: public runtime_error
{
  public:
    string text;
    SrcLocationRange location;

    IRGenException(string text, SrcLocationRange location)
    : location(std::move(location)), text(std::move(text)),
      runtime_error(text.c_str())
    {}

    virtual const char* what() const throw() {
      return text.c_str();
    }
};


/**
 * Internal Exception of the IRGenerator, this error occures not because a faulty malin program but a bug in the ir generation.
 */
class IRGenInternalException: public IRGenException
{
  public:
    IRGenInternalException(string text, SrcLocationRange location)
    : IRGenException(std::move(text), location)
    {}
};


