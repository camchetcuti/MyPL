//----------------------------------------------------------------------
// FILE: ast_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Header file for ast_parser alrogithm.
//----------------------------------------------------------------------

#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"
#include "ast.h"


class ASTParser
{
public:

  // crate a new recursive descent parer
  ASTParser(const Lexer& lexer);

  // run the parser
  Program parse();
  
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
  void struct_def(Program& p);
  void fun_def(Program& p);
  void fields(StructDef& structdef);
  void params(FunDef& fundef);
  bool data_type(VarDef& vardef, VarRef* varref = NULL);
  bool data_type(FunDef& fundef);
  bool base_type();
  void stmt(std::vector<std::shared_ptr<Stmt>>& stmts);
  void vdecl_stmt(std::shared_ptr<VarDeclStmt> vardecstmt1, bool eat_id = false);
  void vdecl_stmt(VarDeclStmt& vardecstmt1, bool eat_id = false);
  void assign_stmt(std::shared_ptr<AssignStmt> assignstmt, bool prev_ID = false);
  void assign_stmt(AssignStmt& assignstmt, bool prev_ID = false);
  void lvalue(std::shared_ptr<AssignStmt>& assignstmt, bool prev_ID = false);
  void lvalue(AssignStmt& assignstmt, bool prev_ID = false);
  void if_stmt(std::shared_ptr<IfStmt>& ifstmt);
  void if_stmt_t(std::shared_ptr<IfStmt>& ifstmt);
  void while_stmt(std::shared_ptr<WhileStmt>& whilestmt);
  void for_stmt(std::shared_ptr<ForStmt>& forstmt);
  bool call_expr(std::shared_ptr<CallExpr> callexpr, bool prev_ID = false);
  bool call_expr2(std::shared_ptr<CallExpr> callexpr, bool prev_ID = false);
  void ret_stmt(std::shared_ptr<ReturnStmt>& returnstmt);
  void expr(Expr& expression, bool prev_ID = false);
  void expr(std::shared_ptr<Expr>& expression, bool prev_ID = false, bool eat_RParen = false);
  bool rvalue(std::shared_ptr<SimpleTerm> simple_term, bool prev_ID = false);
  bool new_rvalue(std::shared_ptr<NewRValue> new_r_val);
  bool base_rvalue();
  bool var_rvalue(std::shared_ptr<VarRValue> var_r_val, bool* prev_ID);

};


#endif
