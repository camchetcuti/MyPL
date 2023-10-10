//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: A program which provides simple parsing for myPl.
//----------------------------------------------------------------------

#include "simple_parser.h"
#include <iostream>

SimpleParser::SimpleParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}


void SimpleParser::eat(TokenType t, const std::string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void SimpleParser::error(const std::string& msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def();
    else if(base_type() || match(TokenType::VOID_TYPE)){
      fun_def();
    }
    else{
      error("expected struct or function definition");
    }
  }
  eat(TokenType::EOS, "expecting end-of-file");
}

// STRUCT DEFINITION
void SimpleParser::struct_def(){
  eat(TokenType::STRUCT, "expecting struct");
  eat(TokenType::ID, "expecting struct ID");
  eat(TokenType::LBRACE, "expecting {");
  fields();
  eat(TokenType::RBRACE, "expecting }");
}

// FUNCTION DEFINITION
void SimpleParser::fun_def(){
  if(data_type() || match(TokenType::VOID_TYPE)){
    if(match(TokenType::VOID_TYPE)){
      advance();
    }
    eat(TokenType::ID, "expected ID2");
    eat(TokenType::LPAREN, "expected (");
    params();
    eat(TokenType::RPAREN, "expected )");
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt();
    }
    eat(TokenType::RBRACE, "expected }");
  }
}

// STRUCT DATA FIELDS
void SimpleParser::fields(){
  if(data_type()){
    eat(TokenType::ID, "expected ID3");
    while(match(TokenType::COMMA)){
      advance();
      if(!data_type()){
        error("expected type");
      }
      eat(TokenType::ID, "expected ID4");
    }
    while(match(TokenType::COMMA)){
      advance();
      if(data_type()){
        eat(TokenType::ID, "expected ID5");
      }
      else{
        error("expected type");
      }
    }
  }
}

// PARAMETERS
void SimpleParser::params(){
  if(data_type()){
    eat(TokenType::ID, "expected ID6");
    while(match(TokenType::COMMA)){
      advance();
      if(!data_type()){
        error("expected type");
      }
      eat(TokenType::ID, "expected ID7");
    }
  }
}

//  DATA TYPES
bool SimpleParser::data_type(){
  if(base_type()){
    advance();
    return true;
  }
  if(match(TokenType::ID)){
    advance();
    return true;
  }
  if(match(TokenType::LIST)){
    advance();
    return true;
  }
  if(match(TokenType::ARRAY)){
    advance();
    if(base_type()){
      advance();
      return true;
    }
    if(match(TokenType::ID)){
      advance();
      return true;
    }
    error("expected type or array ID");
  }
  return false;
}

// BASE TYPES
bool SimpleParser::base_type(){
  return match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, 
                TokenType::CHAR_TYPE, TokenType::STRING_TYPE});
}

// STATEMENT
void SimpleParser::stmt(){
  bool base_type_flag = false;
  if(base_type()){
    base_type_flag = true;
  }
  if(data_type()){
    if(!match(TokenType::DOT) && !match(TokenType::LBRACKET) && !match(TokenType::LPAREN)){
      if(match(TokenType::ID)){
        vdecl_stmt(true);
      }
      else{
        vdecl_stmt(base_type_flag);
      }
    }
    else{
      if(match(TokenType::LPAREN)){
        call_expr(true);
      }
      else{
        assign_stmt(true);
      }
    }
  }
  else if(match(TokenType::IF)){
    if_stmt();
  }
  else if(match(TokenType::WHILE)){
    while_stmt();
  }
  else if(match(TokenType::FOR)){
    for_stmt();
  }
  else if(match(TokenType::RETURN)){
    ret_stmt();
  }
  else{
    error("expected statment");
  }
}

// VARIABLE DECLARATION STATEMENT
void SimpleParser::vdecl_stmt(bool eat_id){
  if(eat_id){
    eat(TokenType::ID, "expected ID");
  }
  eat(TokenType::ASSIGN, "expected =");
  expr(true);
  if(match(TokenType::LPAREN)){
    expr(true);
  }
}

// ASSIGN STATEMENT
void SimpleParser::assign_stmt(bool prev_ID){
  lvalue(prev_ID);
  eat(TokenType::ASSIGN, "expected =");
  expr();
}

// L VALUES
void SimpleParser::lvalue(bool prev_ID){
  if(!prev_ID){
    eat(TokenType::ID, "expected ID9");
  }
  while(match(TokenType::DOT) || match(TokenType::LBRACKET)){
    if(match(TokenType::DOT)){
      advance();
      eat(TokenType::ID, "expected ID10");
    }
    if(match(TokenType::LBRACKET)){
      advance();
      expr();
      eat(TokenType::RBRACKET, "expected }");
    }
  }
}

// IF STATMENTS
void SimpleParser::if_stmt(){
  eat(TokenType::IF, "expected if");
  eat(TokenType::LPAREN, "expected (");
  expr();
  eat(TokenType::RPAREN, "expected ) ");
  eat(TokenType::LBRACE, "expected {"); 
  while(!match(TokenType::RBRACE)){
    stmt();
  }
  eat(TokenType::RBRACE, "expected }");
  if_stmt_t();
}

// IF STATMENT ELSE AND ELSIF
void SimpleParser::if_stmt_t(){
  //ELSEIF
  if(match(TokenType::ELSEIF)){
    advance();
    eat(TokenType::LPAREN, "expected (");
    expr();
    eat(TokenType::RPAREN, "expected )");
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt();
    }
    eat(TokenType::RBRACE, "expected }");
    if_stmt_t();
  }
  // ELSE
  if(match(TokenType::ELSE)){
    advance();
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt();
    }
    eat(TokenType::RBRACE, "expected }");
  }
}

// WHILE STATMENTS
void SimpleParser::while_stmt(){
  eat(TokenType::WHILE, "expecting while");
  eat(TokenType::LPAREN, "expecting (");
  expr();
  eat(TokenType::RPAREN, "expecting )");
  eat(TokenType::LBRACE, "expecting {");
  while(!match(TokenType::RBRACE)){
    stmt();
  }
  eat(TokenType::RBRACE, "expecting }"); 
}

// FOR STATMENTS
void SimpleParser::for_stmt(){
  eat(TokenType::FOR, "expecting for");
  eat(TokenType::LPAREN, "expecting (");
  if(data_type()){
    vdecl_stmt(true);
  }
  else{
    error("expected type");
  }
  eat(TokenType::SEMICOLON, "expecting ;");
  expr();
  eat(TokenType::SEMICOLON, "expecting ;");
  assign_stmt();
  eat(TokenType::RPAREN, "expecting )");
  eat(TokenType::LBRACE, "expecting {");
  while(!match(TokenType::RBRACE)){
    stmt();
  }
  eat(TokenType::RBRACE, "expecting }");
}

// FUNCTION CALLS
bool SimpleParser::call_expr(bool prev_ID){
  if(!prev_ID){
    eat(TokenType::ID, "expected ID");
  }
  if(match(TokenType::NOT)){
    return false;
  }
  eat(TokenType::LPAREN, "expected (");
  if(!match(TokenType::RPAREN)){
    expr(prev_ID);
    while(match(TokenType::COMMA)){
      advance();
      expr();
    }
  }
  eat(TokenType::RPAREN, "expected )");
  return true;
}

// RETURN STATEMENTS
void SimpleParser::ret_stmt(){
  eat(TokenType::RETURN, "expecting return");
  expr(true);
}

// EXPRESSIONS
void SimpleParser::expr(bool prev_ID){
  if(rvalue(prev_ID)){
    if(bin_op()){
      advance();
      expr(true);
    }
  }
  else if(match(TokenType::NOT)){
    advance();
    expr(true);
    if(bin_op()){
      advance();
      expr(true);
    }
  }
  else if(match(TokenType::LPAREN)){
    advance();
    expr();
    eat(TokenType::RPAREN, "expected )");
    if(bin_op()){
      advance();
      expr();
    }
  }
}

// R VALUES
bool SimpleParser::rvalue(bool prev_ID){
  // this is gonna need to have bools returned to determine if
  // the token stream is at an r value
  if(base_rvalue()){
    advance();
    return true;
  }
  else if(match(TokenType::NULL_VAL)){
    advance();
    return true;
  }
  else if(new_rvalue()){
    return true;
  }
  else if(var_rvalue(&prev_ID)){
    return true;
  }
  else if(call_expr(prev_ID)){
    return true;
  }
  else{
    return false;
  }
}

// NEW R VALUE
bool SimpleParser::new_rvalue(){
  if(match(TokenType::NEW)){
    advance();
    if(match(TokenType::ID)){
      advance();
      if(match(TokenType::LBRACKET)){
        advance();
        expr();
        eat(TokenType::RBRACKET, "expecting ]");
      }
      return true;
    }
    if(base_type()){
      advance();
      eat(TokenType::LBRACKET, "expecting [");
      expr();
      eat(TokenType::RBRACKET, "expecting ]");
      return true;
    }
  }
  return false;
}

// BASE RVALUE
bool SimpleParser::base_rvalue(){
  return match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL,
                TokenType::CHAR_VAL, TokenType::STRING_VAL});
}

// VAR R VALUE
bool SimpleParser::var_rvalue(bool* prev_ID){
  if(match(TokenType::ID)){
    advance();
    // Handles function calls
    if(match(TokenType::LPAREN)){
      *prev_ID = true;
      return false;
    }
    while(match(TokenType::DOT) || match(TokenType::LBRACKET)){
      if(match(TokenType::DOT)){
        advance();
        eat(TokenType::ID, "expected ID");
      }
      else if(match(TokenType::LBRACKET)){
        advance();
        expr();
        eat(TokenType::RBRACKET, "expected ]");
      }
    }
    return true;
  }
  return false;
}


