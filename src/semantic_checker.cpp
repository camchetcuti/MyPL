//----------------------------------------------------------------------
// FILE: semantic_checker.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Provides semantic checking for mypl
//----------------------------------------------------------------------

#include <unordered_set>
#include "mypl_exception.h"
#include "semantic_checker.h"
#include <iostream>


using namespace std;

// hash table of names of the base data types and built-in functions
const unordered_set<string> BASE_TYPES {"int", "double", "char", "string", "bool"};
const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat", "list_create", "list_add", "list_numi",
  "list_numd", "list_nums", "list_numb", "list_rmb", "list_avgi", "list_avgd", 
  "list_change", "list_size"};


// helper functions

optional<VarDef> SemanticChecker::get_field(const StructDef& struct_def,
                                            const string& field_name)
{
  for (const VarDef& var_def : struct_def.fields)
    if (var_def.var_name.lexeme() == field_name)
      return var_def;
  return nullopt;
}


void SemanticChecker::error(const string& msg, const Token& token)
{
  string s = msg;
  s += " near line " + to_string(token.line()) + ", ";
  s += "column " + to_string(token.column());
  throw MyPLException::StaticError(s);
}


void SemanticChecker::error(const string& msg)
{
  throw MyPLException::StaticError(msg);
}


// visitor functions


void SemanticChecker::visit(Program& p)
{
  // record each struct def
  for (StructDef& d : p.struct_defs) {
    string name = d.struct_name.lexeme();
    if (struct_defs.contains(name))
      error("multiple definitions of '" + name + "'", d.struct_name);
    struct_defs[name] = d;
  }
  // record each function def (need a main function)
  bool found_main = false;
  for (FunDef& f : p.fun_defs) {
    string name = f.fun_name.lexeme();
    if (BUILT_INS.contains(name))
      error("redefining built-in function '" + name + "'", f.fun_name);
    if (fun_defs.contains(name))
      error("multiple definitions of '" + name + "'", f.fun_name);
    if (name == "main") {
      if (f.return_type.type_name != "void")
        error("main function must have void type", f.fun_name);
      if (f.params.size() != 0)
        error("main function cannot have parameters", f.params[0].var_name);
      found_main = true;
    }
    fun_defs[name] = f;
  }
  if (!found_main)
    error("program missing main function");
  // check each struct
  for (StructDef& d : p.struct_defs)
    d.accept(*this);
  // check each function
  for (FunDef& d : p.fun_defs)
    d.accept(*this);
}


void SemanticChecker::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL)
    curr_type = DataType {false, "int"};
  else if (v.value.type() == TokenType::DOUBLE_VAL)
    curr_type = DataType {false, "double"};    
  else if (v.value.type() == TokenType::CHAR_VAL)
    curr_type = DataType {false, "char"};    
  else if (v.value.type() == TokenType::STRING_VAL)
    curr_type = DataType {false, "string"};    
  else if (v.value.type() == TokenType::BOOL_VAL)
    curr_type = DataType {false, "bool"};    
  else if (v.value.type() == TokenType::NULL_VAL)
    curr_type = DataType {false, "void"};    
}

void SemanticChecker::visit(FunDef& f){
  if(!BASE_TYPES.contains(f.return_type.type_name)){
    if(f.return_type.type_name != "void" && f.return_type.type_name != "list"){
      if(!struct_defs.contains(f.return_type.type_name)){
        error("Function return type is undefined struct", f.fun_name);
      }
    }
  }
  DataType return_type = f.return_type;
  symbol_table.push_environment();
  symbol_table.add("return", return_type);
  for (auto p: f.params){
    if(symbol_table.name_exists_in_curr_env(p.var_name.lexeme())){
      error("Parameter already defined", f.fun_name);
    }
    if(!BASE_TYPES.contains(p.data_type.type_name)){
      if(!struct_defs.contains(p.data_type.type_name)){
        error("Struct parameter not defined", p.var_name);
      }
    }
    symbol_table.add(p.var_name.lexeme(), p.data_type);
  }
  for (auto s: f.stmts){
    s -> accept(*this);
  }
}


void SemanticChecker::visit(StructDef& s){
  symbol_table.push_environment();
  for (VarDef& v : s.fields){
    if (!BASE_TYPES.contains(v.data_type.type_name)){
      // List typing support
      if (v.data_type.type_name != "list"){
        if (!struct_defs.contains(v.data_type.type_name)){
          error("Use of struct before definition", v.var_name);
        }
      }
    }
    if (symbol_table.name_exists_in_curr_env(v.var_name.lexeme())){
      error("Field already defined", v.var_name);
    }
    else{
      symbol_table.add(v.var_name.lexeme(), v.data_type);
    }
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ReturnStmt& s){
  s.expr.accept(*this);
  if(curr_type.type_name != symbol_table.get("return") -> type_name){
    if(curr_type.type_name != "void"){
      error("Invalid return type"); // come up with better error message here
    }
  }
}


void SemanticChecker::visit(WhileStmt& s){
  s.condition.accept(*this);
  if(curr_type.type_name != "bool" || curr_type.is_array != false){
    error("Invalid if conditon type");
  }
  symbol_table.push_environment();
  for(auto st: s.stmts){
    st -> accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ForStmt& s){
  symbol_table.push_environment();
  s.var_decl.accept(*this);
  if(curr_type.type_name != "int" || curr_type.is_array != false){
    error("Incorrect use of for, variable declaration not of type int.");
  }
  s.condition.accept(*this);
  if(curr_type.type_name != "bool" || curr_type.is_array != false){
    error("Incorrect use of for, condtional statement not of type bool.");
  }
  s.assign_stmt.accept(*this);
  if(curr_type.type_name != "int" || curr_type.is_array != false){
    error("Incorrect use of for, interative statement not of type int.");
  }
  symbol_table.push_environment();
  for(auto st: s.stmts){
    st -> accept(*this);
  }
  symbol_table.pop_environment();
  symbol_table.pop_environment();
}


void SemanticChecker::visit(IfStmt& s){
  // Basic ifs
  s.if_part.condition.accept(*this); 
  if(curr_type.type_name != "bool" || curr_type.is_array != false){
    error("Incorrect if condition type, expected bool");
  }
  symbol_table.push_environment();
  for(auto st: s.if_part.stmts){
    st -> accept(*this);
  }
  symbol_table.pop_environment();
  // Else ifs
  for(auto b: s.else_ifs){
    b.condition.accept(*this);
    if(curr_type.type_name != "bool" || curr_type.is_array != false){
      error("Incorrect else if condition type, expected bool");
    }
    symbol_table.push_environment();
    for(auto st: b.stmts){
      st -> accept(*this);
    }
    symbol_table.pop_environment();
  }
  // Else
  symbol_table.push_environment();
  for(auto e: s.else_stmts){
    e -> accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(VarDeclStmt& s){
  s.expr.accept(*this);
  if(curr_type.type_name != s.var_def.data_type.type_name || curr_type.is_array != s.var_def.data_type.is_array){
    if(curr_type.type_name != "void"){
      error("Type mismatch", s.var_def.var_name);
    }
  }
  if(symbol_table.name_exists_in_curr_env(s.var_def.var_name.lexeme())){
    error("Variable already defined", s.var_def.var_name);
  }
  symbol_table.add(s.var_def.var_name.lexeme(), s.var_def.data_type);
}


void SemanticChecker::visit(AssignStmt& s){
  DataType left_type = {false, ""};
  int refrence_length = s.lvalue.size();
  bool return_array = true;
  if(refrence_length == 1){
    if(s.lvalue[0].array_expr != nullopt){
      s.lvalue[0].array_expr -> accept(*this);
      if(curr_type.type_name != "int" || curr_type.is_array != false){
        error("Incorrect indexing type, expected int.", s.lvalue[0].var_name);
      }
      return_array = false;
    }
    if(!symbol_table.name_exists(s.lvalue[0].var_name.lexeme())){
      error("Variable is undefined", s.lvalue[0].var_name);
    }
    std::optional<DataType> var_type = symbol_table.get(s.lvalue[0].var_name.lexeme());
    if (return_array){
      return_array = var_type -> is_array;
    }
    left_type = {return_array, var_type -> type_name};
  }
  // Structs for left side
  else{
    std::optional<VarDef> curr_struct = std::nullopt;
    // path[i] is a VarRef
    bool found_first = false;
    for(int i = 0; i < refrence_length; i++){
      return_array = true;
      if(s.lvalue[i].array_expr != nullopt){
        s.lvalue[i].array_expr -> accept(*this);
        if(curr_type.type_name != "int" || curr_type.is_array != false){
          error("Incorrect indexing type, expected int.", s.lvalue[i].var_name);
        }
        return_array = false;
      }
      if(!symbol_table.name_exists(s.lvalue[i].var_name.lexeme()) && curr_struct == std::nullopt){
        error("Variable is undefined", s.lvalue[i].var_name);
      }
      std::optional<DataType> var_type = symbol_table.get(s.lvalue[i].var_name.lexeme());
      if (return_array){
        return_array = var_type -> is_array;
      }
      if(!BASE_TYPES.contains(var_type -> type_name)){
        if(struct_defs.contains(var_type -> type_name) || found_first){
          if (i != refrence_length - 1){
            if (found_first){
              curr_struct = get_field(struct_defs.at(curr_struct -> data_type.type_name), s.lvalue[i+1].var_name.lexeme());
            }
            else{
              curr_struct = get_field(struct_defs.at(var_type -> type_name), s.lvalue[i+1].var_name.lexeme());
              found_first = true;
            }
          }
          else{
            left_type = {return_array, curr_struct -> data_type.type_name};
            //left_type = {curr_struct -> data_type.is_array, curr_struct -> data_type.type_name};
          }
        }
      }
      else{
        left_type = {return_array, var_type -> type_name};
      }
    } 
  }
  s.expr.accept(*this);
  if(curr_type.type_name != left_type.type_name || curr_type.is_array != left_type.is_array){
    error("Type mismatch at assignment", s.lvalue[0].var_name);
  }
}


void SemanticChecker::visit(CallExpr& e){
  string fun_name = e.fun_name.lexeme();
  // LIST RETRIEVE
  if (fun_name.substr(0, 5) == "list<" && fun_name.substr((fun_name.length() - 9), 9) == ">retrieve"){
      if(e.args.size() != 2){
        error("Invalid number of parameters (expected 2)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid first parameter type (expected list)", e.fun_name);
      }
      e.args[1].accept(*this);
      if (curr_type.type_name != "int"){
        error("Invalid second parameter type (expected int)", e.fun_name);
      }
      curr_type = {false, fun_name.substr(5, (fun_name.length() - 14))};
    }
  // BUILT INS
  else if(BUILT_INS.count(fun_name) > 0){
    // PRINT
    if (fun_name == "print"){
      if(e.args.size() > 1 || e.args.size() == 0){
        error("Invalid number of parameters", e.fun_name);
      }
      e.args[0].accept(*this);
      if (BASE_TYPES.count(curr_type.type_name) == 0 || curr_type.is_array == true){
        error("Invalid parameter type", e.fun_name);
      }
      curr_type = {false, "void"};
    }
    // INPUT
    else if (fun_name == "input"){
      if(e.args.size() != 0){
        error("Invalid number of parameters (expected 0)", e.fun_name);
      }
      curr_type = {false, "string"};
    }
    // TO STRING
    else if (fun_name == "to_string"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(BASE_TYPES.count(curr_type.type_name) > 0){
        if(curr_type.type_name == "bool"){
          error("to_string does not support bool types", e.fun_name);
        }
        curr_type = {false, "string"};
      }
      else{
        error("Invalid parameter type (expected base type)", e.fun_name);
      }
    }
    // TO INT
    else if (fun_name == "to_int"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(BASE_TYPES.count(curr_type.type_name) > 0){
        if(curr_type.type_name == "int"){
          error("to_int does not support int types", e.fun_name);
        }
        curr_type = {false, "int"};
      }
      else{
        error("Invalid parameter type (expected base type)", e.fun_name);
      }
    }
    // TO DOUBLE
    else if (fun_name == "to_double"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(BASE_TYPES.count(curr_type.type_name) > 0){
        if(curr_type.type_name == "double"){
          error("to_double does not support double types", e.fun_name);
        }
        curr_type = {false, "double"};
      }
      else{
        error("Invalid parameter type (expected base type)", e.fun_name);
      }
    }
    // LENGTH
    else if (fun_name == "length"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(curr_type.type_name == "string" && curr_type.is_array == false){
        curr_type = {false, "int"};
      }
      else if (curr_type.is_array == true){
        Token temp = Token(e.fun_name.type(), "length@array", e.fun_name.line(), e.fun_name.column());
        e.fun_name = temp;
        curr_type = {false, "int"};
      }
      else{
        error("Invalid parameter type (expected array or string)", e.fun_name);
      }
    }
    // GET
    else if (fun_name == "get"){
      if(e.args.size() != 2){
        error("Invalid number of parameters (expected 2)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(curr_type.type_name != "int"){
        error("Invalid first parameter type (expected int)", e.fun_name);
      }
      e.args[1].accept(*this);
      if(curr_type.type_name != "string"){
        error("Invalid second parameter type (expected string)", e.fun_name);
      }
      curr_type = {false, "char"};
    }
    // CONCAT
    else if (fun_name == "concat"){
      if(e.args.size() != 2){
        error("Invalid number of parameters (expected 2)", e.fun_name);
      }
      e.args[0].accept(*this);
      if(curr_type.type_name != "string"){
        error("Invalid first parameter type (expected string)", e.fun_name);
      }
      e.args[1].accept(*this);
      if(curr_type.type_name != "string"){
        error("Invalid second parameter type (expected string)", e.fun_name);
      }
    }
    // LIST_CREATE
    else if (fun_name == "list_create"){
      if(e.args.size() != 0){
        error("Invalid number of parameters (expected 0)", e.fun_name);
      }
      curr_type = {false, "list"};
    }
    // LIST_ADD
    else if (fun_name == "list_add"){
      if(e.args.size() != 2){
        error("Invalid number of parameters (expected 2)", e.fun_name);
      }
      e.args[1].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "void"};
    }
    // LIST_NUMI
    else if (fun_name == "list_numi"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
    // LIST_NUMD
    else if (fun_name == "list_numd"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
    // LIST_NUMS
    else if (fun_name == "list_nums"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
    // LIST_NUMB
    else if (fun_name == "list_numb"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
    // LIST_RMB
    else if (fun_name == "list_rmb"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "void"};
    }
    // LIST_AVGI
    else if (fun_name == "list_avgi"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
    // LIST_AVGD
    else if (fun_name == "list_avgd"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "double"};
    }
    // LIST_CHANGE
    else if (fun_name == "list_change"){
      if(e.args.size() != 3){
        error("Invalid number of parameters (expected 3)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid first parameter type (expected list)", e.fun_name);
      }
      e.args[1].accept(*this);
      if (curr_type.type_name != "int"){
        error("Invalid second parameter type (expected int)", e.fun_name);
      }
      curr_type = {false, "void"};
    }
    // LIST_SIZE
    else if (fun_name == "list_size"){
      if(e.args.size() != 1){
        error("Invalid number of parameters (expected 1)", e.fun_name);
      }
      e.args[0].accept(*this);
      if (curr_type.type_name != "list"){
        error("Invalid parameter type (expected list)", e.fun_name);
      }
      curr_type = {false, "int"};
    }
  }
  // USER DEFINED FUNCTIONS
  else{
    std::unordered_map<std::string,FunDef>::const_iterator fun = fun_defs.find(e.fun_name.lexeme());
    if(fun == fun_defs.end()){
      error("Function not defined", e.fun_name);
    }
    else{
      FunDef curr_func = fun -> second;
      // CHECKING NUMBER OF PARAMS
      if(curr_func.params.size() != e.args.size()){
        error("Incorrect number of parameters", e.fun_name);
      }
      // CHECKING PARAMETER TYPES
      for(int i = 0; i < curr_func.params.size(); i++){
        e.args[i].accept(*this);
        if(curr_type.type_name != curr_func.params[i].data_type.type_name || curr_type.is_array != curr_func.params[i].data_type.is_array){
          string error_message = "Invalid parameter type for parameter " + i;
          error(error_message, e.fun_name);
        }
      }
      curr_type = curr_func.return_type;
    }
    
  }
}


void SemanticChecker::visit(Expr& e){
  e.first -> accept(*this);
  DataType lhs_type = curr_type;
  if(e.op.has_value()){
    e.rest -> accept(*this);
    DataType rhs_type = curr_type;
    string op = e.op -> lexeme();
    // STANDARD MATH OPS
    if(op == "+" || op == "-" || op == "*" || op == "/"){
      if(lhs_type.type_name == rhs_type.type_name){
        if(lhs_type.type_name == "int"){
          curr_type = {false, "int"};
        }
        else if(lhs_type.type_name == "double"){
          curr_type = {false, "double"};
        }
        else{
          Token err_token = *e.op;
          error("Incorrect type for operator, expected int or double", err_token);
        }
      }
    }
    // EQUALS OPS
    else if(op == "==" || op == "!="){
      if(lhs_type.type_name == rhs_type.type_name || lhs_type.type_name == "void" || rhs_type.type_name == "void"){
        curr_type = {false, "bool"};
      }
      else{
        Token err_token = *e.op;
        error("Incorrect type for operator, expected same types or void", err_token);
      }
    }
    // COMPARATIVE OPS
    else if(op == "<" || op == ">" || op == "<=" || op == ">="){
      if(lhs_type.type_name == rhs_type.type_name){
        if(lhs_type.type_name == "int" || lhs_type.type_name == "double" || lhs_type.type_name == "char" || lhs_type.type_name == "string"){
          curr_type = {false, "bool"};
        }
        else{
          Token err_token = *e.op;
          error("Incorrect types used; char, string, int, and double supported.", err_token);
        }
      }
      else{
        Token err_token2 = *e.op;
        error("Types must match on either side of comparator", err_token2);
      }
    }
    // BOOL OPERATORS (OR, AND)
    else if(op == "or" || op == "and"){
      if(lhs_type.type_name == "bool" && rhs_type.type_name == "bool"){
        curr_type = {false, "bool"};
      }
      else{
        Token err_token3 = *e.op;
        error("Incorrect types, only booleans supported", err_token3);
      }
    }
    // BOOL OPERATORS (NOT)
    else if(op == "not"){
      if(lhs_type.type_name == "bool"){
        curr_type = {false, "bool"};
      }
      else{
        Token err_token4 = *e.op;
        error("Incorrect type, only bool supported", err_token4);
      }
    }
  }
}


void SemanticChecker::visit(SimpleTerm& t){
  t.rvalue -> accept(*this);
} 


void SemanticChecker::visit(ComplexTerm& t){
  t.expr.accept(*this);
}


void SemanticChecker::visit(NewRValue& v){
  bool is_array = false;
  if(v.array_expr != nullopt){
    is_array = true;
    v.array_expr -> accept(*this);
  }
  curr_type = {is_array, v.type.lexeme()};
}


void SemanticChecker::visit(VarRValue& v){
  bool return_array = true;
  if(v.path.size() == 1){
    if(v.path[0].array_expr != nullopt){
      v.path[0].array_expr -> accept(*this);
      if(curr_type.type_name != "int" || curr_type.is_array != false){
        error("Incorrect indexing type, expected int.", v.path[0].var_name);
      }
      return_array = false;
    }
    if(!symbol_table.name_exists(v.path[0].var_name.lexeme())){
      error("Variable is undefined", v.path[0].var_name);
    }
    std::optional<DataType> var_type = symbol_table.get(v.path[0].var_name.lexeme());
    if (return_array){
      return_array = var_type -> is_array;
    }
    curr_type = {return_array, var_type -> type_name};
  }
  // Struct values
  else{
    std::optional<VarDef> curr_struct = std::nullopt;
    bool found_first = false;
    // path[i] is a VarRef
    for(int i = 0; i < v.path.size(); i++){
      if(v.path[i].array_expr != nullopt){
        v.path[i].array_expr -> accept(*this);
        if(curr_type.type_name != "int" || curr_type.is_array != false){
          error("Incorrect indexing type, expected int.", v.path[i].var_name);
        }
        return_array = false;
      }
      if(!symbol_table.name_exists(v.path[i].var_name.lexeme()) && curr_struct == std::nullopt){
        error("Variable is undefined", v.path[i].var_name);
      }
      std::optional<DataType> var_type = symbol_table.get(v.path[i].var_name.lexeme());
      if (return_array){
        return_array = var_type -> is_array;
      }
      if(!BASE_TYPES.contains(var_type -> type_name)){
        if(struct_defs.contains(var_type -> type_name)){
          if (i != v.path.size() - 1){
            if (found_first){
              curr_struct = get_field(struct_defs.at(curr_struct -> data_type.type_name), v.path[i+1].var_name.lexeme());
            }
            else{
              curr_struct = get_field(struct_defs.at(var_type -> type_name), v.path[i+1].var_name.lexeme());
              found_first = true;
            }
          }
          else{
            curr_type = {return_array, curr_struct -> data_type.type_name};
          }
        }
      }
      else{
        curr_type = {return_array, var_type -> type_name};
      }
    } 
  }
}    

