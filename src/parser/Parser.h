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

    TOKEN_TYPE getNextTokenType() {
      if (tokenIter == tokens.end()) {
        return EndOfFile;
      }
      else {
        return next(tokenIter)->type;
      }
    }

    bool tokensEmpty() {
      return tokenIter == tokens.end() || getTokenType() == EndOfFile;
    }

    void throwUnexpectedTokenException(const vector<TOKEN_TYPE>& expectedTokens, string phaseText = "") {
      string preText = phaseText.empty() ? "" : phaseText + " ";
      if (expectedTokens.empty()) {
        throw ParseException(preText + "got unexpected token " + toString(tokenIter->type), *tokenIter);
      }
      else if (expectedTokens.size() == 1) {
        throw ParseException(preText + "expected " + toString(expectedTokens[0]) + " but got token " + toString(tokenIter->type), *tokenIter);
      }
      else {
        throw ParseException(preText + "expected one of " + toString(expectedTokens) + " but got token " + toString(tokenIter->type), *tokenIter);
      }
    }

    void throwUnexpectedTokenExceptionStr(string expected) {
      throw ParseException("expected " + expected + " but got token " + toString(tokenIter->type), *tokenIter);
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
      if (getTokenType() == Keyword_let) {
        statement = parseVariableDeclaration();
      }
      else {
        statement = parseExpression();
        consumeToken(Semicolon);
        //throwUnexpectedTokenExceptionStr("variable declaration or expression");
      }

      return move(statement);
    }





    /********************************************************
     **** Declarations ***************************************
     */

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





    /********************************************************
     **** Expressions ***************************************
     */

    unique_ptr<Expression> parseExpression() {
      unique_ptr<Expression> exprLHS = parsePrimaryExpression();

      if (!exprLHS) {
        return nullptr;
      }

      // this will return exprLHS if its not a binary expression
      return parseBinaryExpressionRHS(move(exprLHS), 0);
    }

    /**
     * Expression that not contains binary expressions directly.
     * Examples for Primary expressions: "(...)" or "a".
     * Examples for Non Primary: "a + b" or "a * b + c ...".
     */
    unique_ptr<Expression> parsePrimaryExpression() {
      unique_ptr<Expression> expr;

      if (getTokenType() == Identifier) {
        expr = parseIdentifierExpression();
      }
      else if (getTokenType() == Number) {
        expr = parseNumberExpression();
      }
      else if (getTokenType() == String) {
        expr = parseStringExpression();
      }
      else if (getTokenType() == LeftParen) {
        expr = parseParenExpression();
      }
      else {
        throwUnexpectedTokenException({
                Identifier,
                Number,
                String,
                LeftParen,
            }, "at expression");
      }

      return move(expr);
    }

    /**
     * @return nullptr when brace contains no expression
     */
    unique_ptr<Expression> parseParenExpression() {
      unique_ptr<Expression> expr = nullptr;

      consumeToken(LeftParen);
      if (getTokenType() != RightParen) {
        expr = parseExpression();
      }
      consumeToken(RightParen);

      return move(expr);
    }

    unique_ptr<Expression> parseIdentifierExpression()
    {
      // if next token is '(' is a function call
      if (getNextTokenType() == LeftParen)
      {
        auto call = make_unique<CallExpression>();
        call->calledName = consumeToken(Identifier)->contend;

        // arguments
        consumeToken(LeftParen);
        bool gotNamedArgument = false;
        while (!tokensEmpty() && getTokenType() != RightParen)
        {
          auto arg = CallExpressionArgument();
          // parse argument
          // if current is identifier
          // and next is '=' its a named argument
          if (getTokenType() == Identifier &&
              getNextTokenType() == Operator_Assign)
          {
            arg.argName = consumeToken(Identifier)->contend;
            consumeToken(Operator_Assign);
            gotNamedArgument = true;
          }
          // non named arguments are not allowed after named
          else if(gotNamedArgument) {
            throw ParseException("unnamed arguments are not allowed after a named arguments of a function call", *tokenIter);
          }

          // now value expression
          arg.expression = parseExpression();
          call->arguments.push_back(move(arg));

          // comma between params
          if (getTokenType() == Comma){
            consumeToken(Comma);
          }
          else {
            break;
          }
        }
        consumeToken(RightParen);
        return call;
      }

      // otherwise its a variable expression
      else {
        auto variable = make_unique<VariableExpression>();
        variable->name = consumeToken(Identifier)->contend;
        return variable;
      }
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
     * parses chain of "a + b * c - d ..."
     * until it reaches a non binop expression or next binary op precedence is lower than precedenceHigherThan.
     * This will respect operator precedence.
     * If it is not a binary expression like 'a' it will return given lhs expression.
     * @param lhsExpression a already parsed expression, in example above this would be 'a'
     * @param precedenceHigherThan has to be >= 0, next binary ops are only parsed if there precedence is higher than that
     */
    unique_ptr<Expression> parseBinaryExpressionRHS(unique_ptr<Expression> lhsExpression, int precedenceHigherThan) {

      // parse binary expression chain until its end
      while (true)
      {
        // return lhs if its not a binary expression
        // or lastBinOp is stronger then current currentBinOp
        BinaryExpressionOp currentBinOp = BinaryExpression::tokenToBinaryExpressionOp(getTokenType());
        if (currentBinOp == Expr_Op_Invalid || currentBinOp < precedenceHigherThan)
        {
          return lhsExpression;
        }

        // now its a binOp that is stronger then last
        // so consume the operator
        consumeToken(getTokenType());

        // now parse rhs
        auto rhsExpression = parsePrimaryExpression();

        // check if there is a next binary op
        // if there is one and its binding is stronger then currentBinOp
        // -> a new binOp is constructed with rhs and all following rhs with stronger binding than current
        // -> then current rhs is set to that new binary op
        BinaryExpressionOp nextBinOp = BinaryExpression::tokenToBinaryExpressionOp(getTokenType());
        if (nextBinOp != Expr_Op_Invalid && currentBinOp < nextBinOp)
        {
          // parse all following binary ops with stronger precedence than current
          rhsExpression = parseBinaryExpressionRHS(move(rhsExpression), currentBinOp + 1);
        }

        // merge lhs and rhs
        // -> then it becomes the new lhs
        auto newBinary = make_unique<BinaryExpression>();
        newBinary->lhs = move(lhsExpression);
        newBinary->rhs = move(rhsExpression);
        newBinary->operation = currentBinOp;
        lhsExpression = move(newBinary);
      }


      auto expr = make_unique<BinaryExpression>();
      //expr->value = consumeToken(String)->contend;
      return move(expr);
    }
};


