#pragma once

#include <iostream>
#include "exceptions.h"
#include "AST.h"
#include "util/util.h"
#include "SetAstNodeParentAndSelfPass.h"

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


    /**
     * Parse a whole file with the tokens of this file.
     * @return the parsed asts root node
     * @throws ParseException when an error occurs while parsing
     */
    RootDeclarations parse() {
      if (tokens.empty()) {
        throw ParseException("can't parse empty file without any token", Token());
      }

      RootDeclarations root;
      root.location = tokens.begin()->location;

      // for all global declarations
      // this can be:
      // - a global variable declaration
      // - a global function declaration
      tokenIter = tokens.begin();
      while (!tokensEmpty()) {
        // check type of declaration
        if (isVariableDeclaration()) {
          root.variableDeclarations.push_back(parseVariableDeclaration());
        }
        else if (isFunctionDeclaration()) {
          root.functionDeclarations.push_back(parseFunctionDeclaration());
        }
        else if (getTokenType() == Keyword_class) {
          root.classDeclarations.push_back(parseClassDeclaration());
        }
        else {
          throw ParseException("got unexpected token " + toString(tokenIter->type), *tokenIter);
        }
      }
      consumeToken(EndOfFile);

      // set parents of all nodes
      SetAstNodeParentAndSelfPass nodeParentAndSelfPass;
      nodeParentAndSelfPass.run(root);

      return root;
    }



  private:
    /**
     * Consume next expected token.
     * If next token not matches given expected token type, a exception is thrown.
     * @param type the type of next token has to match this type
     * @return iterator to consumed token
     */
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

    /**
     * Consume next expected token.
     * If next token not matches given expected token type, a exception is thrown.
     * @param type the type of next token has to match this type
     * @param applyLocationTo sets the location of this ast node to the consumed tokens location
     * @return iterator to consumed token
     */
    TokenIterator consumeToken(TOKEN_TYPE type, ASTNode &applyLocationTo) {
      auto iter = consumeToken(type);
      applyLocationTo.location = iter->location;
      return iter;
    }

    /**
     * Get token type of the next token that can be consumed.
     * (last consumed token) -> (next token)
     *                           ^^^ return type of this
     */
    TOKEN_TYPE getTokenType() {
      return tokenIter->type;
    }

    /**
     * Get token source location of the next token that can be consumed.
     */
    SrcLocationRange getTokenLocation() {
      return tokenIter->location;
    }

    /**
     * Get the token type of the token after the next token that can be consumed.
     * (last consumed token) -> (next token) -> (next next token)
     *                                           ^^^ return type of this
     */
    TOKEN_TYPE getNextTokenType() {
      if (tokenIter == tokens.end()) {
        return EndOfFile;
      }
      else {
        return next(tokenIter)->type;
      }
    }

    /**
     * Return true if there is no token left for parsing
     */
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





    /********************************************************
     **** Statements ***************************************
     */
    unique_ptr<Statement> parseStatement() {
      unique_ptr<Statement> statement;

      // check statement type
      if (getTokenType() == Keyword_let) {
        statement = parseVariableDeclaration();
      }
      else if (getTokenType() == Keyword_return) {
        statement = parseReturnStatement();
      }
      else if (getTokenType() == Keyword_if) {
        statement = parseIfStatement();
      }
      else if (getTokenType() == Keyword_while) {
        statement = parseWhileStatement();
      }
      else if (getTokenType() == LeftBrace) {
        statement = parseCompoundStatement();
      }
      // otherwise its an expression
      else {
        auto expr = parseExpression();
        // check if its a variable assignment (IdentifierExpression followed by '=')
        auto varExpr = dynamic_cast<VariableExpression*>(expr.get());
        if (varExpr && getTokenType() == Operator_Assign) {
          expr.release();
          statement = parseVariableAssignStatement(unique_ptr<VariableExpression>(varExpr));
        }
        else {
          statement = move(expr);
        }
        consumeToken(Semicolon);
        //throwUnexpectedTokenExceptionStr("variable declaration or expression");
      }

      return move(statement);
    }


    unique_ptr<ReturnStatement> parseReturnStatement() {
      unique_ptr<ReturnStatement> ret = make_unique<ReturnStatement>();
      consumeToken(Keyword_return, *ret);

      // optional expression
      if (getTokenType() != Semicolon) {
        ret->expression = parseExpression();
      }
      consumeToken(Semicolon);
      return move(ret);
    }


    unique_ptr<CompoundStatement> parseCompoundStatement() {
      unique_ptr<CompoundStatement> comp = make_unique<CompoundStatement>();
      consumeToken(LeftBrace, *comp);
      while (!tokensEmpty() && getTokenType() != RightBrace)
      {
        comp->statements.push_back(parseStatement());
      }
      consumeToken(RightBrace);
      return comp;
    }

    unique_ptr<IfStatement> parseIfStatement() {
      unique_ptr<IfStatement> ifSt = make_unique<IfStatement>();
      consumeToken(Keyword_if, *ifSt);

      // condition
      //   consumeToken(LeftParen);
      ifSt->condition = parseExpression();
      //   consumeToken(RightParen);

      // bodies
      ifSt->ifBody = parseCompoundStatement();
      if (getTokenType() == Keyword_else) {
        consumeToken(Keyword_else);
        ifSt->elseBody = parseCompoundStatement();
      }

      return ifSt;
    }

    unique_ptr<WhileStatement> parseWhileStatement() {
      unique_ptr<WhileStatement> whileSt = make_unique<WhileStatement>();

      consumeToken(Keyword_while, *whileSt);
      whileSt->condition = parseExpression();
      whileSt->body = parseCompoundStatement();

      return whileSt;
    }


    /**
     * Parse a VariableAssignStatement but with the variable expression already parsed.
     * The semicolon at the end will not be parsed.
     * Thus only the valueExpression is consumed.
     */
    unique_ptr<VariableAssignStatement> parseVariableAssignStatement(unique_ptr<VariableExpression> variableExpr) {
      unique_ptr<VariableAssignStatement> assign = make_unique<VariableAssignStatement>();

      // variable expression
      auto identifierToken = tokenIter;
      assign->variableExpression = move(variableExpr);

      consumeToken(Operator_Assign, *assign);
      assign->valueExpression = parseExpression();

      return assign;
    }





    /********************************************************
     **** Declarations ***************************************
     */

     /**
      * Parse a variable declaration.
      * @param parentClass if its a member of a class this links to the parent class, then no initial 'let' token is consumed.
      */
    unique_ptr<VariableDeclaration> parseVariableDeclaration(ClassDeclaration *parentClass = nullptr) {
      unique_ptr<VariableDeclaration> var = make_unique<VariableDeclaration>();

      if (!parentClass) {
        consumeToken(Keyword_let);
      } else {
        var->parentClass = parentClass;
      }
      var->name = consumeToken(Identifier, *var)->contend;

      // @todo end location not perfect
      var->location.end = getTokenLocation().getLastLocation();

      // optional type
      if (getTokenType() == Colon) {
        consumeToken(Colon);
        var->typeName = consumeToken(Identifier)->contend;
      }

      // init value
      // optional if it is class member
      if (!parentClass || getTokenType() == Operator_Assign) {
        consumeToken(Operator_Assign);
        var->initExpression = parseExpression();
      }

      consumeToken(Semicolon);
      return move(var);
    }

    /**
     * Parse a function declaration.
     * @param parentClass if its a member of a class this links to the parent class.
     */
    FunctionDeclaration parseFunctionDeclaration(ClassDeclaration *parentClass = nullptr) {
      FunctionDeclaration func;

      if (parentClass) {
        func.parentClass = parentClass;
      }

      consumeToken(Keyword_fun, func);

      // optional extern
      bool isExtern = getTokenType() == Keyword_extern;
      func.isExtern = isExtern;
      if (isExtern) {
        consumeToken(Keyword_extern);
      }
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
      if (!isExtern) {
        func.body = parseCompoundStatement();
      }

      return move(func);
    }

    /**
     * Consumes tokens for FunctionParamDeclaration.
     * It not consumes ending comma.
     */
    FunctionParamDeclaration parseFunctionParamDeclaration() {
      FunctionParamDeclaration param;

      auto paramToken = *tokenIter;
      param.name = consumeToken(Identifier, param)->contend;

      // type
      consumeToken(Colon);
      param.typeName = consumeToken(Identifier)->contend;

      // optional init value
      if (getTokenType() == Operator_Assign) {
        consumeToken(Operator_Assign);
        param.defaultExpression = dynamic_unique_pointer_cast<ConstValueExpression>(parseExpression());
        if (!param.defaultExpression) {
          throw ParseException("only const values are supported for default function arguments", paramToken);
        }
      }

      return move(param);
    }


    /**
     * Class declaration with member vars and functions.
     */
    unique_ptr<ClassDeclaration> parseClassDeclaration() {
      unique_ptr<ClassDeclaration> classDecl = make_unique<ClassDeclaration>();

      consumeToken(Keyword_class, *classDecl);
      classDecl->name = consumeToken(Identifier)->contend;

      // member vars and functions
      consumeToken(LeftBrace);
      while (!tokensEmpty() && getTokenType() != RightBrace)
      {
        if (getTokenType() == Keyword_fun) {
          classDecl->functionDeclarations.push_back(parseFunctionDeclaration(classDecl.get()));
        }
        else if (getTokenType() == Identifier) {
          classDecl->variableDeclarations.push_back(parseVariableDeclaration(classDecl.get()));
        }
        else {
          throwUnexpectedTokenException({
              Keyword_fun,
              Identifier
            }, "class declaration");
        }
      }
      consumeToken(RightBrace);

      return move(classDecl);
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
      else if ((getTokenType() == Number) || (getTokenType() == Operator_Minus && getNextTokenType() == Number)) {
        expr = parseNumberExpression();
      }
      else if (getTokenType() == Keyword_true) {
        expr = make_unique<BoolExpression>(true);
        consumeToken(Keyword_true, *expr);
      }
      else if (getTokenType() == Keyword_false) {
        expr = make_unique<BoolExpression>(false);
        consumeToken(Keyword_false, *expr);
      }
      else if (getTokenType() == String) {
        expr = parseStringExpression();
      }
      else if (getTokenType() == Operator_Unary_Not) {
        expr = parseUnaryNotExpression();
      }
      else if (getTokenType() == LeftParen) {
        expr = parseParenExpression();
      }
      else {
        throwUnexpectedTokenException({
                Identifier,
                Number,
                Operator_Minus,
                String,
                LeftParen,
                Keyword_true,
                Keyword_false,
            }, "expression");
      }

      return move(expr);
    }

    /**
     * Parses '(...)'.
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


    unique_ptr<Expression> parseUnaryNotExpression() {
      unique_ptr<UnaryExpression> expr = make_unique<UnaryExpression>();

      consumeToken(Operator_Unary_Not, *expr);
      expr->operation = Expr_Unary_Op_LOGIC_NOT;
      expr->innerExpression = parsePrimaryExpression();

      return move(expr);
    }

    /**
     * A variable expression like 'myVar'
     * or call like 'func()'
     * or member expression like 'myObject.memberProp'.
     */
    unique_ptr<IdentifierExpression> parseIdentifierExpression(unique_ptr<IdentifierExpression> previousMemberExpr = nullptr)
    {
      unique_ptr<IdentifierExpression> identifierExpr;

      // if next token is '(' is a function call
      if (getNextTokenType() == LeftParen) {
        // if has parent expression -> member call
        if (previousMemberExpr) {
          auto callExpr = dynamic_cast<MemberCallExpression*>(parseCallExpression(true).release());
          auto memberCall = unique_ptr<MemberCallExpression>(callExpr);
          memberCall->parent = move(previousMemberExpr);
          identifierExpr = move(memberCall);
        }
        else {
          identifierExpr = parseCallExpression();
        }
      }
      // otherwise its a variable expression
      else {
        // if has parent expression -> member variable
        if (previousMemberExpr) {
          auto member = make_unique<MemberVariableExpression>();
          member->name = consumeToken(Identifier, *member)->contend;
          member->parent = move(previousMemberExpr);
          identifierExpr = move(member);
        }
        else {
          auto variable = make_unique<VariableExpression>();
          variable->name = consumeToken(Identifier, *variable)->contend;
          identifierExpr = move(variable);
        }
      }

      // has follow expression
      if (getTokenType() == Dot) {
        consumeToken(Dot);
        return parseIdentifierExpression(move(identifierExpr));
      }
      else {
        return identifierExpr;
      }
    }



    unique_ptr<CallExpression> parseCallExpression(bool isMemberCall = false)
    {
      unique_ptr<CallExpression> call;
      if (isMemberCall) {
        call = make_unique<MemberCallExpression>();
      } else {
        call = make_unique<CallExpression>();
      }
      call->calledName = consumeToken(Identifier, *call)->contend;

      // arguments
      consumeToken(LeftParen);
      bool gotNamedArgument = false;
      while (!tokensEmpty() && getTokenType() != RightParen)
      {
        auto arg = CallExpressionArgument();
        arg.location = getTokenLocation();

        bool namedArg = false;
        // parse argument
        // if current is identifier
        // and next is '=' or ':' its a named argument
        if (getTokenType() == Identifier &&
            (getNextTokenType() == Operator_Assign || getNextTokenType() == Colon))
        {
          arg.argName = consumeToken(Identifier)->contend;
          if (getTokenType() == Operator_Assign) {
            consumeToken(Operator_Assign);
          } else {
            consumeToken(Colon);
          }
          gotNamedArgument = true;
          namedArg = true;
        }
        // non named arguments are not allowed after named
        else if(gotNamedArgument) {
          throw ParseException("unnamed arguments are not allowed after named arguments of a function call", *tokenIter);
        }

        // now value expression
        arg.expression = parseExpression();
        if (namedArg) {
          call->argumentsNamed.push_back(move(arg));
        } else {
          call->argumentsNonNamed.push_back(move(arg));
        }


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


    unique_ptr<Expression> parseStringExpression() {
      auto expr = make_unique<StringExpression>();
      expr->value = consumeToken(String, *expr)->contend;
      return move(expr);
    }

    unique_ptr<Expression> parseNumberExpression() {
      unique_ptr<NumberExpression> expr;
      string contend;
      Token numberToken;

      bool isNegative = getTokenType() == Operator_Minus;
      if (isNegative) {
        consumeToken(Operator_Minus);
        numberToken = *consumeToken(Number);
        contend = "-" + numberToken.contend;
      }
      else {
        numberToken = *consumeToken(Number);
        contend = numberToken.contend;
      }

      try {
        // if its a integer
        if (contend.find('.') == string::npos) {
          auto exprInt = make_unique<NumberIntExpression>();
          exprInt->value = stoi(contend);
          exprInt->location = numberToken.location;
          expr = move(exprInt);
        }
          // when its a floating point
        else {
          auto exprFloat = make_unique<NumberFloatExpression>();
          exprFloat->value = stof(contend);
          exprFloat->location = numberToken.location;
          expr = move(exprFloat);
        }
      }
      catch(exception &e) {
        throw ParseException(string("can't convert NumberExpression to number: ") + e.what(), numberToken);
      }

      return move(expr);
    }

    /**
     * Parses chain of "a + b * c - d ..."
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
        auto opLocation = consumeToken(getTokenType())->location;

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
        newBinary->location = opLocation;
        lhsExpression = move(newBinary);
      }


      auto expr = make_unique<BinaryExpression>();
      //expr->value = consumeToken(String)->contend;
      return move(expr);
    }
};


