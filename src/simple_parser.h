//----------------------------------------------------------------------
// FILE: simple_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: Basic interface for our Simple Parser (syntax checker)
//----------------------------------------------------------------------

#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"


class SimpleParser
{
public:

  // crate a new recursive descent parer
  SimpleParser(const Lexer& lexer);

  // run the parser
  void parse();
  
private:
  
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, const std::string& msg);
  bool match(TokenType t);
  bool match(std::initializer_list<TokenType> types);
  void error(const std::string& msg);
  bool bin_op();

  // recursive descent functions
  void struct_def();
  void fun_def();
  void fields();
  void params();
  bool data_type();
  bool base_type();
  void stmt();
  void vdecl_stmt(bool eat_id = false);
  void assign_stmt(bool prev_ID = false);
  void lvalue(bool prev_ID = false);
  void if_stmt();
  void if_stmt_t();
  void while_stmt();
  void for_stmt();
  bool call_expr(bool prev_ID = false);
  void ret_stmt();
  void expr(bool prev_ID = false);
  bool rvalue(bool prev_ID = false);
  bool new_rvalue();
  bool base_rvalue();
  bool var_rvalue(bool* prev_ID);
  
};


#endif
