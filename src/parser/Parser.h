#pragma once

#include <iostream>
#include "exceptions.h"
#include "AST.h"
#include "util/util.h"

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
      while (!tokensEmpty()) {
        // check type of declaration
        if (isVariableDeclaration()) {
          root.variableDeclarations.push_back(parseVariableDeclaration());
        }
        else if (isFunctionDeclaration()) {
          root.functionDeclarations.push_back(parseFunctionDeclaration());
        }
        else {
          throw ParseException("got unexpected token " + toString(tokenIter->type), *tokenIter);
        }
      }
      consumeToken(EndOfFile);

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
      }
      else {
        // @todo lexer needs to create a end of file token by itself
        Token eofToken = Token();
        eofToken.type = EndOfFile;
        throw ParseException("expected token " + toString(type) + " but reached end of file", eofToken);
      }
    }

    TOKEN_TYPE getTokenType() {
      return tokenIter->type;
    }

    bool tokensEmpty() {
      return tokenIter == tokens.end() || getTokenType() == EndOfFile;
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
      return tokenIter->type == Keyword_fun;
    }

    unique_ptr<Statement> parseStatement() {
      unique_ptr<Statement> statement;

      // check statement type
      if (isVariableDeclaration()) {
        statement = parseVariableDeclaration();
      }

      return move(statement);
    }

    unique_ptr<VariableDeclaration> parseVariableDeclaration() {
      unique_ptr<VariableDeclaration> var = make_unique<VariableDeclaration>();

      consumeToken(Keyword_let);
      var->name = consumeToken(Identifier)->contend;

      // optional type
      if (getTokenType() == Colon) {
        consumeToken(Colon);
        var->typeName = consumeToken(Identifier)->contend;
      }

      // init value
      consumeToken(Operator_Assign);
      var->initExpression = parseExpression();

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
      unique_ptr<NumberExpression> expr;

      string contend = consumeToken(Number)->contend;
      try {
        // if its a integer
        if (contend.find('.') == string::npos) {
          auto exprInt = make_unique<NumberIntExpression>();
          exprInt->value = stoi(contend);
          expr = move(exprInt);
        }
        // when its a floating point
        else {
          auto exprFloat = make_unique<NumberFloatExpression>();
          exprFloat->value = stof(contend);
          expr = move(exprFloat);
        }
      }
      catch(exception &e) {
        throw ParseException(string("can't convert NumberExpression to number: ") + e.what(), *tokenIter);
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



    FunctionDeclaration parseFunctionDeclaration() {
      FunctionDeclaration func;

      consumeToken(Keyword_fun);
      func.name = consumeToken(Identifier)->contend;

      // arguments
      consumeToken(LeftParen);
      while (!tokensEmpty() && getTokenType() != RightParen)
      {
        func.arguments.push_back(parseFunctionParamDeclaration());

        // comma between params
        if (getTokenType() == Comma){
          consumeToken(Comma);
        }
        else {
          break;
        }
      }
      consumeToken(RightParen);

      // optional return type
      if (getTokenType() == Colon) {
        consumeToken(Colon);
        func.typeName = consumeToken(Identifier)->contend;
      }
        // if no return type -> use void
      else {
        func.typeName = "void";
      }

      // body
      consumeToken(LeftBrace);
      while (!tokensEmpty() && getTokenType() != RightBrace)
      {
        func.bodyStatements.push_back(parseStatement());
      }
      consumeToken(RightBrace);

      return move(func);
    }

    /**
     * Consumes tokens for FunctionParamDeclaration.
     * It not consumes ending comma.
     */
    FunctionParamDeclaration parseFunctionParamDeclaration() {
      FunctionParamDeclaration param;

      param.name = consumeToken(Identifier)->contend;

      // type
      consumeToken(Colon);
      param.typeName = consumeToken(Identifier)->contend;

      // optional init value
      if (getTokenType() == Operator_Assign) {
        consumeToken(Operator_Assign);
        param.defaultExpression = parseExpression();
      }

      return move(param);
    }
};


