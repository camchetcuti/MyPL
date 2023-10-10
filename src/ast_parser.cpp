//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Parsing alogirthm for MyPl that builds an AST while executing.
//----------------------------------------------------------------------

#include "ast_parser.h"
#include <iostream>
using namespace std;


ASTParser::ASTParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void ASTParser::advance()
{
  curr_token = lexer.next_token();
}


void ASTParser::eat(TokenType t, const string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void ASTParser::error(const string& msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool ASTParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else
      fun_def(p);
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}


// STRUCT DEFINITION
void ASTParser::struct_def(Program& p){
  StructDef structdef;
  eat(TokenType::STRUCT, "expecting struct");
  structdef.struct_name = curr_token;
  eat(TokenType::ID, "expecting struct ID");
  eat(TokenType::LBRACE, "expecting {");
  fields(structdef);
  eat(TokenType::RBRACE, "expecting }");
  p.struct_defs.push_back(structdef);
}

// FUNCTION DEFINITION
void ASTParser::fun_def(Program& p){
  FunDef fundef;
  if(data_type(fundef) || match(TokenType::VOID_TYPE)){
    if(match(TokenType::VOID_TYPE)){
      DataType voidType;
      voidType.type_name = "void";
      fundef.return_type = voidType;
      advance();
    }
    fundef.fun_name = curr_token;
    eat(TokenType::ID, "expected ID");
    eat(TokenType::LPAREN, "expected (");
    params(fundef);
    eat(TokenType::RPAREN, "expected )");
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt(fundef.stmts);
    }
    eat(TokenType::RBRACE, "expected }");
    p.fun_defs.push_back(fundef);
  }
  else{
    error("expected function declaration");
  }
}

// STRUCT DATA FIELDS
void ASTParser::fields(StructDef& structdef){
  VarDef vardef;
  if(data_type(vardef)){
    vardef.var_name = curr_token;
    eat(TokenType::ID, "expected ID");
    structdef.fields.push_back(vardef);
    while(match(TokenType::COMMA)){
      VarDef loopVarDef;
      advance();
      if(!data_type(loopVarDef)){
        error("expected type");
      }
      loopVarDef.var_name = curr_token;
      eat(TokenType::ID, "expected ID");
      structdef.fields.push_back(loopVarDef);
    }
    while(match(TokenType::COMMA)){
      VarDef loopVarDef;
      advance();
      if(data_type(loopVarDef)){
        loopVarDef.var_name = curr_token;
        eat(TokenType::ID, "expected ID");
        structdef.fields.push_back(loopVarDef);
      }
      else{
        error("expected type");
      }
    }
  }
}

// PARAMETERS
void ASTParser::params(FunDef& fundef){
  VarDef vardef;
  if(data_type(vardef)){
    vardef.var_name = curr_token;
    eat(TokenType::ID, "expected ID");
    fundef.params.push_back(vardef);
    while(match(TokenType::COMMA)){
      VarDef loopVarDef;
      advance();
      if(!data_type(loopVarDef)){
        error("expected type");
      }
      loopVarDef.var_name = curr_token;
      eat(TokenType::ID, "expected ID");
      fundef.params.push_back(loopVarDef);
    }
  }
}

//  DATA TYPES
bool ASTParser::data_type(VarDef& vardef, VarRef* varref){ 
  DataType datatype;                        
  if(base_type()){                      
    datatype.type_name = curr_token.lexeme();
    vardef.data_type =  datatype;
    advance();
    return true;
  }
  if(match(TokenType::LIST)){                     
    datatype.type_name = curr_token.lexeme();
    vardef.data_type =  datatype;
    advance();
    return true;
  }
  if(match(TokenType::ID)){
    datatype.type_name = curr_token.lexeme();
    vardef.data_type =  datatype;
    if(varref != NULL){ // statment added, simple path assign passes as result
      varref -> var_name = curr_token;
    }
    advance();
    return true;
  }
  if(match(TokenType::ARRAY)){
    datatype.is_array = true;
    advance();
    if(base_type()){
      datatype.type_name = curr_token.lexeme();
      vardef.data_type = datatype;
      advance();
      return true;
    }
    if(match(TokenType::ID)){
      datatype.type_name = curr_token.lexeme();
      vardef.data_type = datatype;
      advance();
      return true;
    }
    error("expected type or array ID");
  }
  return false;
}

//  DATA TYPES FOR FUNCTIONS
bool ASTParser::data_type(FunDef& fundef){
  DataType datatype;
  if(base_type()){
    datatype.type_name = curr_token.lexeme();
    fundef.return_type =  datatype;
    advance();
    return true;
  }
  if(match(TokenType::ID)){
    datatype.type_name = curr_token.lexeme();
    fundef.return_type =  datatype;
    advance();
    return true;
  }
  if(match(TokenType::LIST)){
    datatype.type_name = curr_token.lexeme();
    fundef.return_type =  datatype;
    advance();
    return true;
  }
  if(match(TokenType::ARRAY)){
    datatype.is_array = true;
    advance();
    if(base_type()){
      datatype.type_name = curr_token.lexeme();
      fundef.return_type = datatype;
      advance();
      return true;
    }
    if(match(TokenType::ID)){
      datatype.type_name = curr_token.lexeme();
      advance();
      return true;
    }
    error("expected type or array ID");
  }
  return false;
}

// BASE TYPES
bool ASTParser::base_type(){
  return match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, 
                TokenType::CHAR_TYPE, TokenType::STRING_TYPE});
}

// STATEMENT
void ASTParser::stmt(std::vector<std::shared_ptr<Stmt>>& stmts){
  Token startToken = curr_token;
  bool base_type_flag = false;
  VarDef vardef;
  VarRef varref;
  if(base_type()){
    base_type_flag = true;
  }
  if(data_type(vardef, &varref)){
    if(vardef.data_type.is_array || !match(TokenType::ASSIGN)){
      base_type_flag = true;
    }
    if(!match(TokenType::DOT) && !match(TokenType::LBRACKET) && !match(TokenType::LPAREN) && base_type_flag){
      std::shared_ptr<VarDeclStmt> vardecstmt1 = std::make_shared<VarDeclStmt>();
      vardecstmt1 -> var_def = vardef;
      if(match(TokenType::ID)){
        vdecl_stmt(vardecstmt1, true);
        stmts.push_back(vardecstmt1);
      }
      else{
        vdecl_stmt(vardecstmt1, base_type_flag);
        stmts.push_back(vardecstmt1);
      }
    }
    else{
      if(match(TokenType::LPAREN)){
        std::shared_ptr<CallExpr> call_expr_ptr = std::make_shared<CallExpr>();
        call_expr_ptr -> fun_name = startToken;
        call_expr2(call_expr_ptr, true);
        stmts.push_back(call_expr_ptr);
      }
      else{
        std::shared_ptr<AssignStmt> assignstmt = std::make_shared<AssignStmt>();
        if(varref.var_name.lexeme() != ""){
          assignstmt -> lvalue.push_back(varref);
        }
        assign_stmt(assignstmt, true);
        stmts.push_back(assignstmt);
      }
    }
  }
  else if(match(TokenType::IF)){
    std::shared_ptr<IfStmt> ifstmt = std::make_shared<IfStmt>();
    if_stmt(ifstmt);
    stmts.push_back(ifstmt);
  }
  else if(match(TokenType::WHILE)){
    std::shared_ptr<WhileStmt> whilestmt = std::make_shared<WhileStmt>();
    while_stmt(whilestmt);
    stmts.push_back(whilestmt);
  }
  else if(match(TokenType::FOR)){
    std::shared_ptr<ForStmt> forstmt = std::make_shared<ForStmt>();
    for_stmt(forstmt);
    stmts.push_back(forstmt);
  }
  else if(match(TokenType::RETURN)){
    std::shared_ptr<ReturnStmt> returnstmt = make_shared<ReturnStmt>();
    ret_stmt(returnstmt);
    stmts.push_back(returnstmt);
  }
  else{
    error("expected statment");
  }
}

// VARIABLE DECLARATION STATEMENT
void ASTParser::vdecl_stmt(std::shared_ptr<VarDeclStmt> vardecstmt1, bool eat_id){
  if(eat_id){
    vardecstmt1 -> var_def.var_name = curr_token;
    eat(TokenType::ID, "expected ID");
  }
  eat(TokenType::ASSIGN, "expected =");
  Expr expression;
  expr(expression, true);
  vardecstmt1 -> expr = expression;
}

// VARIABLE DECLARATION STATEMENT WITHIN FOR LOOP DECLARATION
void ASTParser::vdecl_stmt(VarDeclStmt& vardecstmt1, bool eat_id){
  if(eat_id){
    vardecstmt1.var_def.var_name = curr_token;
    eat(TokenType::ID, "expected ID");
  }
  eat(TokenType::ASSIGN, "expected =");
  Expr expression;
  expr(expression, true);
  vardecstmt1.expr = expression;
}

// ASSIGN STATEMENT
void ASTParser::assign_stmt(std::shared_ptr<AssignStmt> assignstmt, bool prev_ID){
  lvalue(assignstmt, prev_ID);
  eat(TokenType::ASSIGN, "expected =");
  expr(assignstmt -> expr);
}

// ASSIGN STATEMENT WHEN IN FOR LOOP DECLARATION
void ASTParser::assign_stmt(AssignStmt& assignstmt, bool prev_ID){
  lvalue(assignstmt, prev_ID);
  eat(TokenType::ASSIGN, "expected =");
  expr(assignstmt.expr);
}

// L VALUES
void ASTParser::lvalue(std::shared_ptr<AssignStmt>& assignstmt, bool prev_ID){
  bool firstBrack = false;
  if(match(TokenType::LBRACKET)){
    firstBrack = true;
  }
  while(match(TokenType::DOT) || match(TokenType::LBRACKET)){
    VarRef loopVarRef;
    if(match(TokenType::DOT)){
      advance();
      loopVarRef.var_name = curr_token;
      eat(TokenType::ID, "expected ID");
    }
    if(match(TokenType::LBRACKET)){
      if(!firstBrack){
        advance();
        Expr expression;
        expr(expression);    
        loopVarRef.array_expr = expression;
        eat(TokenType::RBRACKET, "expected }");
      }
      else{
        advance();
        Expr expression_first;
        expr(expression_first);    
        assignstmt -> lvalue[0].array_expr = expression_first;
        eat(TokenType::RBRACKET, "expected }");
      }
    }
    if(firstBrack){
      firstBrack = false;
    }
    else{
      assignstmt -> lvalue.push_back(loopVarRef);
    }
  }
}

// L VALUES WHEN IN FOR LOOP DECLARATION
void ASTParser::lvalue(AssignStmt& assignstmt, bool prev_ID){

  bool firstBrack = false;
  if(match(TokenType::LBRACKET)){
    firstBrack = true;
  }
  VarRef first;
  first.var_name = curr_token;
  eat(TokenType::ID, "expected ID");
  if(match(TokenType::LBRACKET)){
    advance();
    Expr expression;
    expr(expression);    
    first.array_expr = expression;
    eat(TokenType::RBRACKET, "expected ]");
  }
  assignstmt.lvalue.push_back(first);

  while(match(TokenType::DOT) || match(TokenType::LBRACKET)){
    VarRef loopVarRef;
    if(match(TokenType::DOT)){
      advance();
      loopVarRef.var_name = curr_token;
      eat(TokenType::ID, "expected ID");
    }
    if(match(TokenType::LBRACKET)){
      if(!firstBrack){
        advance();
        Expr expression;
        expr(expression);    
        loopVarRef.array_expr = expression;
        eat(TokenType::RBRACKET, "expected ]");
      }
      else{
        advance();
        Expr expression_first;
        expr(expression_first);    
        assignstmt.lvalue[0].array_expr = expression_first;
        eat(TokenType::RBRACKET, "expected ]");
      }
    }
    if(firstBrack){
      firstBrack = false;
    }
    else{
      assignstmt.lvalue.push_back(loopVarRef);
    }
  }
}

// IF STATMENTS
void ASTParser::if_stmt(std::shared_ptr<IfStmt>& ifstmt){
  eat(TokenType::IF, "expected if");
  eat(TokenType::LPAREN, "expected (");
  BasicIf basicif;
  expr(basicif.condition);
  eat(TokenType::RPAREN, "expected ) ");
  eat(TokenType::LBRACE, "expected {"); 
  while(!match(TokenType::RBRACE)){
    stmt(basicif.stmts);
  }
  eat(TokenType::RBRACE, "expected }");
  ifstmt -> if_part = basicif;
  if_stmt_t(ifstmt);
}

// IF STATMENT ELSE AND ELSIF
void ASTParser::if_stmt_t(std::shared_ptr<IfStmt>& ifstmt){
  //ELSEIF
  if(match(TokenType::ELSEIF)){
    advance();
    eat(TokenType::LPAREN, "expected (");
    BasicIf basicif;
    expr(basicif.condition);
    eat(TokenType::RPAREN, "expected )");
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt(basicif.stmts);
    }
    eat(TokenType::RBRACE, "expected }");
    ifstmt -> else_ifs.push_back(basicif);
    if_stmt_t(ifstmt);
  }
  // ELSE
  if(match(TokenType::ELSE)){
    advance();
    eat(TokenType::LBRACE, "expected {");
    while(!match(TokenType::RBRACE)){
      stmt(ifstmt -> else_stmts);
    }
    eat(TokenType::RBRACE, "expected }");
  }
}

// WHILE STATMENTS
void ASTParser::while_stmt(std::shared_ptr<WhileStmt>& whilestmt){
  eat(TokenType::WHILE, "expecting while");
  eat(TokenType::LPAREN, "expecting (");
  expr(whilestmt -> condition);
  eat(TokenType::RPAREN, "expecting )");
  eat(TokenType::LBRACE, "expecting {");
  while(!match(TokenType::RBRACE)){
    stmt(whilestmt -> stmts);
  }
  eat(TokenType::RBRACE, "expecting }"); 
}

// FOR STATMENTS
void ASTParser::for_stmt(std::shared_ptr<ForStmt>& forstmt){
  eat(TokenType::FOR, "expecting for");
  eat(TokenType::LPAREN, "expecting (");
  VarDeclStmt vardecstmt2;
  if(data_type(vardecstmt2.var_def)){
    vdecl_stmt(vardecstmt2, true);
    forstmt -> var_decl = vardecstmt2;
  }
  else{
    error("expected type");
  }
  eat(TokenType::SEMICOLON, "expecting ;");
  expr(forstmt -> condition);
  eat(TokenType::SEMICOLON, "expecting ;");
  assign_stmt(forstmt -> assign_stmt);
  eat(TokenType::RPAREN, "expecting )");
  eat(TokenType::LBRACE, "expecting {");
  while(!match(TokenType::RBRACE)){
    stmt(forstmt -> stmts);
  }
  eat(TokenType::RBRACE, "expecting }");
}

// FUNCTION CALLS
bool ASTParser::call_expr(std::shared_ptr<CallExpr> callexpr, bool prev_ID){
  if(!prev_ID && !match(TokenType::NOT) && !match(TokenType::LPAREN)){
    callexpr -> fun_name = curr_token;
    eat(TokenType::ID, "expected ID");
  }
  if(match(TokenType::NOT) || (match(TokenType::LPAREN) && !prev_ID)){
    return false;
  }
  
  eat(TokenType::LPAREN, "expected (");
  if(!match(TokenType::RPAREN)){
    Expr expression;
    expr(expression, prev_ID);
    callexpr -> args.push_back(expression); 
    while(match(TokenType::COMMA)){
      advance();
      Expr expression2;
      expr(expression2);
      callexpr -> args.push_back(expression2);
    }
  }
  eat(TokenType::RPAREN, "expected )");
  return true;
}

bool ASTParser::call_expr2(std::shared_ptr<CallExpr> callexpr, bool prev_ID){
  if(match(TokenType::NOT)){
    return false;
  }
  if(!prev_ID){
    callexpr -> fun_name = curr_token;
    eat(TokenType::ID, "expected ID");
  }
  eat(TokenType::LPAREN, "expected (");
  if(!match(TokenType::RPAREN)){
    Expr expression;
    expr(expression, prev_ID);
    callexpr -> args.push_back(expression); 
    while(match(TokenType::COMMA)){
      advance();
      Expr expression2;
      expr(expression2);
      callexpr -> args.push_back(expression2);
    }
  }
  eat(TokenType::RPAREN, "expected )");
  return true;
}

// RETURN STATEMENTS
void ASTParser::ret_stmt(std::shared_ptr<ReturnStmt>& returnstmt){
  eat(TokenType::RETURN, "expecting return");
  expr(returnstmt -> expr, true);
}

// EXPRESSIONS
void ASTParser::expr(Expr& expression, bool prev_ID){
  if (match(TokenType::NOT)){
    expression.negated = true;
    advance();
    expr(expression);
  }
  else if (match(TokenType::LPAREN)){
    std::shared_ptr<ComplexTerm> complex_first = std::make_shared<ComplexTerm>();
    advance();
    expr(complex_first -> expr);
    expression.first = complex_first;
    eat(TokenType::RPAREN, "expecting ) ");
  }
  else {
    std::shared_ptr<SimpleTerm> simple_first = std::make_shared<SimpleTerm>();
    rvalue(simple_first, prev_ID); // THIS MAY REQUIRE A RESTRUCTURE TO DEAL WITH NEW STRUCTURE
    expression.first = simple_first;
  }
  if (bin_op()){
    expression.op = curr_token;
    advance();
    std::shared_ptr<Expr> restexpr = make_shared<Expr>();
    expr(restexpr, prev_ID); 
    expression.rest = restexpr;
  }
}

// EXPRESSIONS WITH SHARED POINTER TYPE
void ASTParser::expr(std::shared_ptr<Expr>& expression, bool prev_ID, bool eat_RParen){
  if (match(TokenType::NOT)){
    expression -> negated = true;
    advance();
    expr(expression);
  }
  else if (match(TokenType::LPAREN)){
    std::shared_ptr<ComplexTerm> complex_first = std::make_shared<ComplexTerm>();
    advance();
    expr(complex_first -> expr);
    expression -> first = complex_first;
    eat(TokenType::RPAREN, "expecting ) ");
  }
  else {
    std::shared_ptr<SimpleTerm> simple_first = std::make_shared<SimpleTerm>();
    rvalue(simple_first, prev_ID); // THIS MAY REQUIRE A RESTRUCTURE TO DEAL WITH NEW STRUCTURE
    expression -> first = simple_first;
  }
  if (bin_op()){
    expression -> op = curr_token;
    advance();
    std::shared_ptr<Expr> restexpr = make_shared<Expr>();
    expr(restexpr, prev_ID); 
    expression -> rest = restexpr;
  }
}

// R VALUES
bool ASTParser::rvalue(std::shared_ptr<SimpleTerm> simple_term, bool prev_ID){
  // this is gonna need to have bools returned to determine if
  // the token stream is at an r value
  std::shared_ptr<NewRValue> new_r_val = std::make_shared<NewRValue>();
  std::shared_ptr<SimpleRValue> simple_r_val = std::make_shared<SimpleRValue>();
  std::shared_ptr<VarRValue> var_r_val = std::make_shared<VarRValue>();
  std::shared_ptr<CallExpr> call_expr_ptr = std::make_shared<CallExpr>();
  if(base_rvalue()){
    simple_r_val -> value = curr_token;
    simple_term -> rvalue = simple_r_val;
    advance();
    return true;
  }
  else if(match(TokenType::NULL_VAL)){
    simple_r_val -> value = curr_token;
    simple_term -> rvalue = simple_r_val;
    advance();
    return true;
  }
  else if(new_rvalue(new_r_val)){
    simple_term -> rvalue = new_r_val;
    return true;
  }
  else if(var_rvalue(var_r_val, &prev_ID)){
    simple_term -> rvalue = var_r_val;
    return true;
  }
  else if(call_expr(call_expr_ptr, prev_ID)){ 
    call_expr_ptr -> fun_name = var_r_val -> path[0].var_name;
    simple_term -> rvalue = call_expr_ptr;
    return true;
  }
  else{
    return false;
  }
}

// NEW R VALUE
bool ASTParser::new_rvalue(std::shared_ptr<NewRValue> new_r_val){
  if(match(TokenType::NEW)){
    advance(); 
    if(match(TokenType::ID)){
      new_r_val -> type = curr_token;
      advance();
      if(match(TokenType::LBRACKET)){
        advance();
        Expr expression;
        expr(expression);
        new_r_val -> array_expr = expression;
        eat(TokenType::RBRACKET, "expecting ]");
      }
      return true;
    }
    if(base_type()){
      Expr expression;
      new_r_val -> type = curr_token;
      advance();
      eat(TokenType::LBRACKET, "expecting [");
      expr(expression);
      new_r_val -> array_expr = expression;
      eat(TokenType::RBRACKET, "expecting ]");
      return true;
    }
  }
  return false;
}

// BASE RVALUE
bool ASTParser::base_rvalue(){
  return match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL,
                TokenType::CHAR_VAL, TokenType::STRING_VAL});
}

// VAR R VALUE
bool ASTParser::var_rvalue(std::shared_ptr<VarRValue> var_r_val, bool* prev_ID){
  VarRef varref;
  if(match(TokenType::ID)){
    varref.var_name = curr_token;
    advance();
    // Handles function calls
    if(match(TokenType::LPAREN)){
      var_r_val -> path.push_back(varref);
      *prev_ID = true;
      return false;
    }
    var_r_val -> path.push_back(varref);
    bool first_through = true;
    while(match(TokenType::DOT) || match(TokenType::LBRACKET)){
      VarRef varref2;
      if(match(TokenType::DOT)){
        advance();
        varref2.var_name = curr_token;
        eat(TokenType::ID, "expected ID");
        if(match(TokenType::LBRACKET)){
          advance();
          Expr expression2;
          expr(expression2);
          varref2.array_expr = expression2;
          eat(TokenType::RBRACKET, "expected ]");
        }
        var_r_val -> path.push_back(varref2);
      }
      else if(match(TokenType::LBRACKET)){
        advance();
        Expr expression;
        expr(expression);
        if(first_through){
          var_r_val -> path[0].array_expr = expression;
          first_through = false;
        }
        eat(TokenType::RBRACKET, "expected ]");
      }
    }
    return true;
  }
  return false;
}

