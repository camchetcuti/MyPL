//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Pretty printer for MyPl.
//----------------------------------------------------------------------

#include "print_visitor.h"
#include <iostream>

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}

// FUNCTION DEFINITIONS
void PrintVisitor::visit(FunDef& f){
  print_indent();
  out << "\n" << f.return_type.type_name << " ";
  out << f.fun_name.lexeme() << "(";
  for(int i = 0; i < f.params.size(); i++){
    out << f.params[i].data_type.type_name << " ";
    if(i == f.params.size()-1){
      out << f.params[i].var_name.lexeme();
    }
    else{
      out << f.params[i].var_name.lexeme() << ", ";
    }
  }
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < f.stmts.size(); i++){
    print_indent();
    f.stmts[i] -> accept(*this);
    out << "\n";
  }
  dec_indent();
  out << "}\n";
}

// STRUCT DEFINITIONS
void PrintVisitor::visit(StructDef& s){
  print_indent();
  out << "\nstruct ";
  out << s.struct_name.lexeme() << " ";
  out << "{\n";
  inc_indent();
  for(int i = 0; i < s.fields.size(); i++){
    print_indent();
    if(s.fields[i].data_type.is_array){
      out << "array ";
    }
    out << s.fields[i].data_type.type_name;
    out << " " << s.fields[i].var_name.lexeme();
    if(i != s.fields.size() - 1){
      out << ",\n";
    }
    else{
      out << "\n";
    }
  }
  dec_indent();
  print_indent();
  out << "\n}\n";
}

// RETURN STATEMENTS
void PrintVisitor::visit(ReturnStmt& s){
  out << "return ";
  s.expr.accept(*this);
}

// WHILE STATEMENTS
void PrintVisitor::visit(WhileStmt& s){
  out << "while (";
  s.condition.accept(*this);
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < s.stmts.size(); i++){
    print_indent();
    s.stmts[i] -> accept(*this);
    out << "\n";
  }
  dec_indent();
  print_indent();
  out << "}";
}

// FOR STATEMENTS
void PrintVisitor::visit(ForStmt& s){
  out << "for (";
  s.var_decl.accept(*this);
  out << "; ";
  s.condition.accept(*this);
  out << "; ";
  s.assign_stmt.accept(*this);
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < s.stmts.size(); i++){
    print_indent();
    s.stmts[i] -> accept(*this);
    out << "\n";
  } 
  dec_indent();
  print_indent();
  out << "}";
}

// IF STATEMENTS
void PrintVisitor::visit(IfStmt& s){
  out << "if (";
  s.if_part.condition.accept(*this);
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < s.if_part.stmts.size(); i++){
    print_indent();
    s.if_part.stmts[i] -> accept(*this);
    out << "\n";
  }
  dec_indent();
  print_indent();
  out << "}";
  for(int i = 0; i < s.else_ifs.size(); i++){
    out << "\n";
    print_indent();
    out << "elseif (";
    s.else_ifs[i].condition.accept(*this);
    out << ") {\n";
    inc_indent();
    for(int j = 0; j < s.else_ifs[i].stmts.size(); j++){
      print_indent();
      s.else_ifs[i].stmts[j] -> accept(*this);
      out << "\n";
    }
    dec_indent();
    print_indent();
    out << "}";
  }
  out << "\n";
  print_indent();
  out << "else {\n";
  inc_indent();
  for(int i = 0; i < s.else_stmts.size(); i++){
    print_indent();
    s.else_stmts[i] -> accept(*this);
    out << "\n";
  }
  dec_indent();
  print_indent();
  out << "}";
}

// VARIABLE DECLARATION STATEMENTS
void PrintVisitor::visit(VarDeclStmt& s){
  if(s.var_def.data_type.is_array){
    out << "array " << s.var_def.data_type.type_name;
  }
  else{
    out << s.var_def.data_type.type_name;
  }
  out << " " << s.var_def.var_name.lexeme();
  out << " = ";
  s.expr.accept(*this);
}

// ASSIGN STATEMENTS
void PrintVisitor::visit(AssignStmt& s){
  for(int i = 0; i < s.lvalue.size(); i++){
    out << s.lvalue[i].var_name.lexeme();
    if(s.lvalue[i].array_expr != std::nullopt){
      out << "[";
      s.lvalue[i].array_expr -> accept(*this);
      out << "]";
    }
    if(i == s.lvalue.size() -1){
      out << " ";
    }
    else{
      out << ".";
    }
  }
  out << "= ";
  s.expr.accept(*this);
}

// FUNCTION CALLS
void PrintVisitor::visit(CallExpr& e){
  out << e.fun_name.lexeme() << "(";
  for(int i = 0; i < e.args.size(); i++){
    e.args[i].accept(*this);
    if(i != e.args.size() -1){
      out << ", ";
    }
    out << "";
  }
  out << ")";
}

// EXPRESSIONS
void PrintVisitor::visit(Expr& e){
  e.first -> accept(*this);
  if(e.op != std::nullopt){
    out << " " << e.op -> lexeme() << " ";
  }
  if (e.rest != nullptr){
    e.rest -> accept(*this);
  }
}

// SIMPLE TERMS
void PrintVisitor::visit(SimpleTerm& t){
  t.rvalue -> accept(*this);
}

// COMPLEX TERMS
void PrintVisitor::visit(ComplexTerm& t){
  out << "(";
  t.expr.accept(*this);
  out << ")";
}

// SIMPLE R VALUES
void PrintVisitor::visit(SimpleRValue& v){
  if(v.value.type() == TokenType::STRING_VAL){
    out << "\"" << v.value.lexeme() << "\"";

  }
  else{
    out << v.value.lexeme();
  }
}

// NEW R VALUES
void PrintVisitor::visit(NewRValue& v){
  out << "new " << v.type.lexeme();
  if(v.array_expr != std::nullopt){
    out << "[";
    v.array_expr -> accept(*this);
    out << "]";
  }
}

// VARIABLE R VALUES
void PrintVisitor::visit(VarRValue& v){
  for(int i = 0; i < v.path.size(); i++){
    out << v.path[i].var_name.lexeme();
    if(v.path[i].array_expr != std::nullopt){
      out << "[";
      v.path[i].array_expr -> accept(*this);
      out << "]";
    }
    if(i != v.path.size() - 1){
      out << ".";
    }
  }
}
