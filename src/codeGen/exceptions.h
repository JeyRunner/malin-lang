#pragma once

#include<iostream>
#include <utility>
using namespace std;


/*
 * exceptions class
 */
class CodeGenException: public runtime_error
{
  public:
    string text;
    SrcLocationRange location;

    CodeGenException(string text, SrcLocationRange location)
    : location(std::move(location)), text(std::move(text)),
      runtime_error(text.c_str())
    {}

    virtual const char* what() const throw() {
      return text.c_str();
    }
};


