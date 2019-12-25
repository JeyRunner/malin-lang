#pragma once

#include<iostream>
#include <utility>
using namespace std;


/*
 * exceptions class
 */
class ParseException: public runtime_error
{
  public:
    Token token;
    string text;

    ParseException(string text, Token token)
    : token(std::move(token)), text(std::move(text)),
      runtime_error(text.c_str())
    {}
};


