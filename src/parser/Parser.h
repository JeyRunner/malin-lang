#pragma once

#include <iostream>
#include "exceptions.h"
#include "AST.h"

using namespace std;


/*
 * Parser class
 */
class Parser
{
  private:
    list<Token> tokens;
    using TokenIterator = list<Token>::iterator;
    TokenIterator tokenIter = tokens.begin();

  public:
    explicit Parser(list<Token> &&tokens) : tokens(move(tokens))
    {}

    RootDeclarations parse() {
      RootDeclarations root;

      // for all declarations
      tokenIter = tokens.begin();
      while (tokenIter != tokens.end()) {
        // check type of declaration
        if (isVariableDeclaration()) {
          root.variableDeclarations.push_back(
              parseVariableDeclaration()
          );
        }
        else if (isFunctionDeclaration()) {
          throw ParseException("func not supported", *tokenIter);
        }
        else {
          throw ParseException("got unexpected token " + toString(tokenIter->type), *tokenIter);
        }
      }

      return root;
    }

  private:
    TokenIterator consumeToken(TOKEN_TYPE type) {
      if (tokenIter != tokens.end()) {
        if (tokenIter->type == type) {
          TokenIterator iterOld = tokenIter;
          tokenIter++;
          return iterOld;
        } else {
          throw ParseException("expected token " + toString(type) + " but got token " + toString(tokenIter->type), *tokenIter);
        }
      } else {
        throw ParseException("expected token " + toString(type) + " but reached end of file", *tokenIter);
      }
    }

    TOKEN_TYPE getTokenType() {
      return tokenIter->type;
    }

    void throwUnexpectedTokenException(const vector<TOKEN_TYPE>& expectedTokens) {
      if (expectedTokens.empty()) {
        throw ParseException("got unexpected token " + toString(tokenIter->type), *tokenIter);
      }
      else if (expectedTokens.size() == 1) {
        throw ParseException("expected " + toString(expectedTokens[0]) + " but got token " + toString(tokenIter->type), *tokenIter);
      }
      else {
        throw ParseException("expected one of " + toString(expectedTokens) + " but got token " + toString(tokenIter->type), *tokenIter);
      }
    }


    bool isVariableDeclaration() {
      return tokenIter->type == Keyword_let;
    }

    bool isFunctionDeclaration() {
      return tokenIter->type == Keyword_func;
    }

    VariableDeclaration parseVariableDeclaration() {
      VariableDeclaration var;

      consumeToken(Keyword_let);
      var.name = consumeToken(Identifier)->contend;

      // optional type
      if (getTokenType() == Colon) {
        consumeToken(Colon);
        var.typeName = consumeToken(Identifier)->contend;
      }

      // init value
      consumeToken(Operator_Assign);
      var.initExpression = parseExpression();

      consumeToken(Semicolon);
      return move(var);
    }

    unique_ptr<Expression> parseExpression() {
      unique_ptr<Expression> expr;

      if (getTokenType() == Identifier) {
        expr = parseVariableExpression();
      }
      else if (getTokenType() == Number) {
        expr = parseNumberExpression();
      }
      else if (getTokenType() == String) {
        expr = parseStringExpression();
      }
      else {
        throwUnexpectedTokenException({
          Identifier,
          Number,
          String,
        });
      }

      return move(expr);
    }

    unique_ptr<Expression> parseVariableExpression() {
      auto expr = make_unique<VariableExpression>();
      expr->name = consumeToken(Identifier)->contend;
      return move(expr);
    }

    unique_ptr<Expression> parseStringExpression() {
      auto expr = make_unique<StringExpression>();
      expr->value = consumeToken(String)->contend;
      return move(expr);
    }

    unique_ptr<Expression> parseNumberExpression() {
      auto expr = make_unique<NumberExpression>();

      string contend = consumeToken(Number)->contend;
      try {
        expr->value = stod(contend);
      } catch(exception &e) {
        throw ParseException(string("can't convert NumberExpression to double: ") + e.what(), *tokenIter);
      }

      return move(expr);
    }

    /**
     * Returns null if given expression is not a binary one
     * @todo parseBinaryExpression
     */
    unique_ptr<Expression> parseBinaryExpression() {
      auto expr = make_unique<BinaryExpression>();
      //expr->value = consumeToken(String)->contend;
      return move(expr);
    }
};


