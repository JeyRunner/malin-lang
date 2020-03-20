#pragma once

#include <iostream>
#include <magic_enum.hpp>
#include <utility>
#include <functional>
#include <list>
#include <optional>
using namespace std;


enum TOKEN_TYPE
{
    Invalid,
    Comment,
    Number,
    String,
    Identifier,
    Semicolon,
    Colon,
    Comma,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Keyword_let,
    Keyword_if,
    Keyword_while,
    Keyword_else,
    Keyword_true,
    Keyword_false,
    Keyword_fun,
    Keyword_extern,
    Keyword_return,
    Operator_Plus,
    Operator_Minus,
    Operator_Multiply,
    Operator_Divide,
    Operator_Assign,
    Operator_Equals,
    Operator_NotEquals,
    Operator_GreaterThen,
    Operator_GreaterEqualThen,
    Operator_LessThen,
    Operator_LessEqualThen,
    Operator_LogicOr,
    Operator_LogicAnd,
    EndOfFile,
};

string toString(TOKEN_TYPE tokenType) {
  return string(magic_enum::enum_name(tokenType));
}

string toString(vector<TOKEN_TYPE> tokenTypes) {
  string s = "[";
  for (int i = 0; i < tokenTypes.size(); ++i)
  {
    s+= toString(tokenTypes[i]);
    if (i < tokenTypes.size() - 1) {
      s+= ", ";
    }
  }
  s+= "]";
  return s;
}


class SrcLocation
{
  public:
    int line;
    int column;
    int index;

    SrcLocation(int line, int columnStart, int absoluteCharIndex)
        : column(columnStart), line(line), index(absoluteCharIndex)
    {
    }

    string toString()
    {
      return "" + to_string(line) + ":" + to_string(column);
    }
};

class SrcLocationRange {
  public:
    SrcLocation start;
    optional<SrcLocation> end;

    explicit SrcLocationRange(const SrcLocation &start) : start(start)
    {
    }

    SrcLocationRange(const SrcLocation &start, const SrcLocation &end) : start(start), end(end)
    {
    }

    SrcLocation getLastLocation() {
      if (end) {
        return *end;
      } else {
        return start;
      }
    }

    string toString()
    {
      if (end) {
        return start.toString() + " to " + end->toString();
      } else {
        return start.toString();
      }
    }
};


class Token
{
  public:
    TOKEN_TYPE type;
    string contend;
    SrcLocationRange location;


    Token(TOKEN_TYPE type, string contend, SrcLocationRange location)
        : type(type), contend(std::move(contend)), location(location)
    {
    }

    Token(TOKEN_TYPE type, char contend, SrcLocationRange location)
        : type(type), contend(1, contend), location(location)
    {
    }

    explicit Token(TOKEN_TYPE type, SrcLocationRange location)
        : type(type), contend(""), location(location)
    {
    }

    Token()
        : type(Invalid), contend(), location(SrcLocation(-1, -1, -1))
    {
    }

    string toString()
    {
      if (contend.length() != 0)
      {
        return "Token( " + string(magic_enum::enum_name(type)) + ", " + contend + " ) at [" + location.toString() + "]";
      }
      else
      {
        return "Token( " + string(magic_enum::enum_name(type)) + " ) at [" + location.toString() + "]";
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
      // @todo size should be text.size()
      // cout << "-- file size " << size << endl;
    }


    list <Token> getAllTokens()
    {
      list <Token> tokenList;
      for (Token token = getNextToken(); !atEndOfFile(); token = getNextToken())
      {
        // ignore comments
        if (token.type != Comment) {
          tokenList.push_back(token);
        }
      }

      // add eof token at end
      // @todo add eof token when it appears in input text
      Token eofToken = Token(EndOfFile, SrcLocationRange(location));
      tokenList.push_back(eofToken);

      return tokenList;
    }


    Token getNextToken()
    {
      // skip spaces
      skipSpaces();

      if (atEndOfFile())
      {
        return Token(EndOfFile, SrcLocationRange(location));
      }


      // when starts identifier
      if (isSymbolChar(getCurrentChar()) == START_OR_INNER_IDENTIFIER_CHAR)
      {
        return makeIdentifierOrKeyword();
      }

      // when starts number
      if (isDigitChar(getCurrentChar()))
      {
        return makeNumber();
      }

      // when starts string
      if (isStringStartEndChar(getCurrentChar()))
      {
        return makeString();
      }


      // single char tokens and comments
      switch (getCurrentChar())
      {
        case '+':
          return makeSingleCharToken(Operator_Plus);
        case '-':
          return makeSingleCharToken(Operator_Minus);
        case '*':
          return makeSingleCharToken(Operator_Multiply);
        case '/':
          if (compareNextCharWith('/')) {
            return makeOneLineComment();
          }
          else if (compareNextCharWith('*')) {
            return makeMultiLineComment();
          }
          else {
            return makeSingleCharToken(Operator_Divide);
          }

        case '=':
          if (compareNextCharWith('=')) {
            return makeDoubleCharToken(Operator_Equals);
          }
          else {
            return makeSingleCharToken(Operator_Assign);
          }
        case '!':
          if (compareNextCharWith('=')) {
            return makeDoubleCharToken(Operator_NotEquals);
          }
        case '>':
          if (compareNextCharWith('=')) {
            return makeDoubleCharToken(Operator_GreaterEqualThen);
          } else {
            return makeSingleCharToken(Operator_GreaterThen);
          }
        case '<':
          if (compareNextCharWith('=')) {
            return makeDoubleCharToken(Operator_LessEqualThen);
          } else {
            return makeSingleCharToken(Operator_LessThen);
          }
        case '|':
          if (compareNextCharWith('|')) {
            return makeDoubleCharToken(Operator_LogicOr);
          }
        case '&':
          if (compareNextCharWith('&')) {
            return makeDoubleCharToken(Operator_LogicAnd);
          }
        case ',':
          return makeSingleCharToken(Comma);
        case ';':
          return makeSingleCharToken(Semicolon);
        case ':':
          return makeSingleCharToken(Colon);
        case '(':
          return makeSingleCharToken(LeftParen);
        case ')':
          return makeSingleCharToken(RightParen);
        case '{':
          return makeSingleCharToken(LeftBrace);
        case '}':
          return makeSingleCharToken(RightBrace);
      }

      // @todo parse end of file

      // fallback
      Token token(Invalid, getCurrentChar(), SrcLocationRange(location));
      nextChar();
      return token;
    }

    bool atEndOfFile()
    {
      return location.index >= size;
    }

  private:
    const char *text;
    const unsigned long size;
    SrcLocation location = SrcLocation(1, 1, 0);


    Token makeSingleCharToken(TOKEN_TYPE type)
    {
      SrcLocationRange l(location);
      nextChar();
      return Token(type, SrcLocationRange(l));
    }

    Token makeDoubleCharToken(TOKEN_TYPE type)
    {
      auto start = location;
      nextChar();
      nextChar();
      auto end = location;
      return Token(type, SrcLocationRange(SrcLocationRange(start, end)));
    }

    Token makeIdentifierOrKeyword()
    {
      SrcLocation start = location;

      // over all identifier chars
      skipCharsWhile([this]()
                     {
                       return isSymbolChar(getCurrentChar()) != NO_IDENTIFIER_CHAR;
                     });
      SrcLocation end = location;

      // char array subpart to string
      string contend = getSubText(start.index, end.index);

      // check if it is keyword
      if (contend == "let"){
        return Token(Keyword_let, SrcLocationRange(start, end));
      }
      if (contend == "if"){
        return Token(Keyword_if, SrcLocationRange(start, end));
      }
      if (contend == "while"){
        return Token(Keyword_while, SrcLocationRange(start, end));
      }
      if (contend == "else"){
        return Token(Keyword_else, SrcLocationRange(start, end));
      }
      if (contend == "true"){
        return Token(Keyword_true, SrcLocationRange(start, end));
      }
      if (contend == "false"){
        return Token(Keyword_false, SrcLocationRange(start, end));
      }
      if (contend == "fun"){
        return Token(Keyword_fun, SrcLocationRange(start, end));
      }
      if (contend == "extern"){
        return Token(Keyword_extern, SrcLocationRange(start, end));
      }
      if (contend == "return"){
        return Token(Keyword_return, SrcLocationRange(start, end));
      }

      // if not a keyword its a identifier
      return Token(Identifier, contend, SrcLocationRange(start, end));
    }


    /**
     * progress "asdf"
     * -> will skip first " and last "
     */
    Token makeString()
    {
      SrcLocation start = location;

      // skip initial "
      nextChar();
      SrcLocation startText = location;

      // over all identifier chars
      skipCharsWhile([this]()
                     {
                       return !isStringStartEndChar(getCurrentChar());
                     });
      SrcLocation endText = location;

      // skip end "
      nextChar();
      SrcLocation end = location;

      // char array subpart to string
      return Token(String, getSubText(startText.index, endText.index), SrcLocationRange(start, end));
    }

    Token makeNumber()
    {
      SrcLocation start = location;
      // first digits
      skipCharsWhile([this]()
                     {
                       return isDigitChar(getCurrentChar());
                     });

      // dot
      if (isDotChar(getCurrentChar()))
      {
        nextChar();

        // get the part after dot
        skipCharsWhile([this]()
                       {
                         return isDigitChar(getCurrentChar());
                       });
      }

      SrcLocation end = location;

      // char array subpart to string
      return Token(Number, getSubText(start.index, end.index), SrcLocationRange(start, end));
    }


    Token makeOneLineComment() {
      SrcLocation start = location;

      // both '/'
      nextChar();
      nextChar();
      SrcLocation startText = location;

      // skip all until next line
      skipCharsWhile([this]()
                     {
                       return isSpaceChar(getCurrentChar()) != NEW_LINE;
                     });

      SrcLocation end = location;
      return Token(Comment, getSubText(startText.index, end.index), SrcLocationRange(start, end));
    }


    Token makeMultiLineComment() {
      SrcLocation start = location;

      // '/*'
      nextChar();
      nextChar();
      SrcLocation startText = location;

      // skip all until next line
      skipCharsWhile([this]()
                     {
                       return !(getCurrentChar() == '*' && compareNextCharWith('/'));
                     });
      SrcLocation endText = location;

      // '*/'
      nextChar();
      nextChar();

      SrcLocation end = location;
      return Token(Comment, getSubText(startText.index, endText.index), SrcLocationRange(start, end));
    }





    void nextChar()
    {
      if (atEndOfFile())
      {
        throw out_of_range(
            "character at position " + location.toString() + " is after file end (file has " + to_string(size)
                + " characters)");
      }

      location.index++;
      location.column++;
    }

    char getCurrentChar()
    {
      return text[location.index];
    }

    bool compareNextCharWith(char compareWith)
    {
      if (atEndOfFile()) {
        return false;
      } else {
        return text[location.index + 1] == compareWith;
      }
    }


    void skipSpaces()
    {
      SPACE_TYPE space = isSpaceChar(getCurrentChar());
      while (space != NO_SPACE && !atEndOfFile())
      {
        if (space == NEW_LINE)
        {
          location.line++;
          location.column = 0;
        }

        nextChar();
        space = isSpaceChar(getCurrentChar());
      }
    }



    static bool isStringStartEndChar(char c)
    {
      return c == '"';
    }

    static bool isDotChar(char c)
    {
      return c == '.';
    }

    enum SPACE_TYPE
    {
        NO_SPACE,
        SPACE,
        NEW_LINE,
    };

    static SPACE_TYPE isSpaceChar(char c)
    {
      switch (c)
      {
        case '\n':
          return NEW_LINE;
        case ' ':
        case '\t':
        case '\r':
          return SPACE;
        default:
          return NO_SPACE;
      }
    }

    static bool isDigitChar(char c)
    {
      switch (c)
      {
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


    enum IDENTIFIER_CHAR_TYPE
    {
        NO_IDENTIFIER_CHAR,
        START_OR_INNER_IDENTIFIER_CHAR,
        INNER_IDENTIFIER_CHAR,
    };

    /**
     * Is the given char an Identifier Char.
     * @return NO_IDENTIFIER_CHAR               if it can't be part of an Identifier
     *         START_OR_INNER_IDENTIFIER_CHAR   if char can be first or a character after the first of an Identifier
     *         INNER_IDENTIFIER_CHAR            if char can be a character after the first of an Identifier
     */
    static IDENTIFIER_CHAR_TYPE isSymbolChar(char c)
    {
      switch (c)
      {
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

    void skipCharsWhile(function<bool()> condition)
    {
      while (condition() && !atEndOfFile())
      {
        if (isSpaceChar(getCurrentChar()) == NEW_LINE)
        {
          location.line++;
          location.column = 0;
        }
        nextChar();
      }
    }
};


