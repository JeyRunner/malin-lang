#pragma once

#include <iostream>
#include <magic_enum.hpp>
#include <utility>
#include <functional>
using namespace std;


enum TOKEN_TYPE {
    Invalid,
    Number,
    String,
    Symbol,
    Semicolon,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Operator_Plus,
    Operator_Minus,
    Operator_Assign,
    EndOfFile,
};

class SrcLocation{
  public:
    int start;
    int end;

    SrcLocation(int start, int end) : start(start), end(end)
    {
    }

    string toString() {
      return "" + to_string(start) + " - " + to_string(end);
    }
};

class Token {
  public:
    TOKEN_TYPE type;
    string contend;
    SrcLocation location;


    Token(TOKEN_TYPE type, string contend, SrcLocation location)
    : type(type), contend(std::move(contend)), location(location)
    {
    }

    Token(TOKEN_TYPE type, char contend, SrcLocation location)
        : type(type), contend(1, contend), location(location)
    {
    }

    explicit Token(TOKEN_TYPE type, SrcLocation location)
        : type(type), contend(""), location(location)
    {
    }

    string toString() {
      if (contend.length() != 0) {
        return "Token( " + string(magic_enum::enum_name(type)) + ", " + contend + " ) at [" + location.toString() +"]";
      } else {
        return "Token( " + string(magic_enum::enum_name(type)) + " ) at [" + location.toString() +"]";
      }
    }
};



/*
 * Lexer class
 */
class Lexer
{
  public:


    explicit Lexer(string &text)
    : text(text.c_str()), size(text.size() + 1)
    {
      cout << "-- file size " << size << endl;
    }


    Token getNextToken() {
      if (atEndOfFile()) {
        return Token(EndOfFile, SrcLocation(0, 0));
      }

      // skip spaces
      while (isSpaceChar(getCurrentChar())) {
        nextChar();
      }


      // when starts identifier
      if (isSymbolChar(getCurrentChar()) == START_OR_INNER_IDENTIFIER_CHAR) {
        return makeSymbol();
      }

      // when starts number
      if (isDigitChar(getCurrentChar())) {
        return makeNumber();
      }

      // when starts string
      if (isStringStartEndChar(getCurrentChar())) {
        return makeString();
      }


      // single char tokens
      switch (getCurrentChar()) {
        case '+':
          return makeSingleCharToken(Operator_Plus);
        case '-':
          return makeSingleCharToken(Operator_Minus);
        case '=':
          return makeSingleCharToken(Operator_Assign);
        case ';':
          return makeSingleCharToken(Semicolon);
        case '(':
          return makeSingleCharToken(LeftParen);
        case ')':
          return makeSingleCharToken(RightParen);
        case '{':
          return makeSingleCharToken(LeftBrace);
        case '}':
          return makeSingleCharToken(RightBrace);
      }


      // fallback
      Token token(Invalid, getCurrentChar(), SrcLocation(0, 0));
      nextChar();
      return token;
    }

    bool atEndOfFile() {
      return index >= size;
    }

  private:
    const char *text;
    const int size;
    int index = 0;


    Token makeSingleCharToken(TOKEN_TYPE type) {
      nextChar();
      return Token(type, SrcLocation(index, index));
    }

    Token makeSymbol() {
      int start = index;

      // over all identifier chars
      skipCharsWhile([this] () {
        return isSymbolChar(getCurrentChar()) != NO_IDENTIFIER_CHAR;
      });
      int end = index;

      // char array subpart to string
      return Token(Symbol, getSubText(start, end), SrcLocation(start, end));
    }


    /**
     * progress "asdf"
     * -> will skip first " and last "
     */
    Token makeString() {
      // skip initial "
      nextChar();
      int start = index;

      // over all identifier chars
      skipCharsWhile([this] () {
        return !isStringStartEndChar(getCurrentChar());
      });
      int end = index;

      // skip end "
      nextChar();

      // char array subpart to string
      return Token(String, getSubText(start, end), SrcLocation(start, end));
    }

    Token makeNumber() {
      int start = index;
      // first digits
      skipCharsWhile([this] () {
        return isDigitChar(getCurrentChar());
      });

      // dot
      if (isDotChar(getCurrentChar())) {
        nextChar();

        // get the part after dot
        skipCharsWhile([this] () {
          return isDigitChar(getCurrentChar());
        });
      }

      int end = index;

      // char array subpart to string
      return Token(Number, getSubText(start, end), SrcLocation(start, end));
    }



    void nextChar() {
      index++;

      if (atEndOfFile()) {
        throw out_of_range("character at position " + to_string(index) + " is after file end (file has " + to_string(size) + " characters)");
      }
    }

    char getCurrentChar() {
      return text[index];
    }



    static bool isStringStartEndChar(char c) {
      return c == '"';
    }

    static bool isDotChar(char c) {
      return c == '.';
    }

    static bool isSpaceChar(char c) {
      switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          return true;
        default:
          return false;
      }
    }

    static bool isDigitChar(char c) {
      switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          return true;
        default:
          return false;
      }
    }


    enum IDENTIFIER_CHAR_TYPE {
        NO_IDENTIFIER_CHAR,
        START_OR_INNER_IDENTIFIER_CHAR,
        INNER_IDENTIFIER_CHAR,
    };

    /**
     * Is the given char an Symbol Char.
     * @return NO_IDENTIFIER_CHAR               if it can't be part of an Symbol
     *         START_OR_INNER_IDENTIFIER_CHAR   if char can be first or a character after the first of an Symbol
     *         INNER_IDENTIFIER_CHAR            if char can be a character after the first of an Symbol
     */
    static IDENTIFIER_CHAR_TYPE isSymbolChar(char c) {
      switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '_':
          return INNER_IDENTIFIER_CHAR;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
          return START_OR_INNER_IDENTIFIER_CHAR;
        default:
          return NO_IDENTIFIER_CHAR;
      }
    }

    string getSubText(int start, int end) const
    {
      string contend;
      contend.assign(text, start, end - start);
      return contend;
    }

    void skipCharsWhile(function<bool()> condition) {
      while (condition() && !atEndOfFile()) {
        nextChar();
      }
    }
};


