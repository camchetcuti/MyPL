//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Code generator for myPl
//----------------------------------------------------------------------

#include <iostream>             // for debugging
#include "code_generator.h"

using namespace std;


// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}


void CodeGenerator::visit(FunDef& f)
{
  VMFrameInfo new_frame;
  curr_frame = new_frame;
  var_table.push_environment();
  curr_frame.function_name = f.fun_name.lexeme();
  curr_frame.arg_count = f.params.size();
  // Generates store instructions for params
  for (int i = 0; i < curr_frame.arg_count; i++){
    var_table.add(f.params[i].var_name.lexeme());
    curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(f.params[i].var_name.lexeme())));
  }
  // Visits the statements
  for (int i = 0; i < f.stmts.size(); i++){
    f.stmts[i] -> accept(*this);
  }
  // Ensures return statment
  if (curr_frame.instructions.size() == 0){
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());
  }
  else if(curr_frame.instructions[-1].opcode() != OpCode::RET){
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());
  }
  // Pops var_table and adds frame
  var_table.pop_environment();
  vm.add(curr_frame);
}


void CodeGenerator::visit(StructDef& s)
{ 
  struct_defs[s.struct_name.lexeme()] = s;
}


void CodeGenerator::visit(ReturnStmt& s)
{
  // pushes value
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::RET());
}


void CodeGenerator::visit(WhileStmt& s)
{ 
  var_table.push_environment();
  // start loop has index of first conditional instruction
  int start_loop = curr_frame.instructions.size();
  // evaluates condtion
  s.condition.accept(*this);
  // jumps to end of loop
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  int loop_end_jmp = curr_frame.instructions.size() - 1;
  // handles statements
  for (int i = 0; i < s.stmts.size(); i++){
    s.stmts[i] -> accept(*this);
  }
  // jumps to start of loop
  curr_frame.instructions.push_back(VMInstr::JMP(start_loop));
  curr_frame.instructions.push_back(VMInstr::NOP());
  // changes the jmpf command to have the correct instruction
  curr_frame.instructions[loop_end_jmp] = VMInstr::JMPF(curr_frame.instructions.size() - 1);
  var_table.pop_environment();
}



void CodeGenerator::visit(ForStmt& s)
{
  // int i = 0
  var_table.push_environment();
  s.var_decl.accept(*this);
  int condition = curr_frame.instructions.size();
  // i < 5
  s.condition.accept(*this);
  int end_loop = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for (int i = 0; i < s.stmts.size(); i++){
      s.stmts[i] -> accept(*this);
  }
  var_table.pop_environment();
  s.assign_stmt.accept(*this);
  curr_frame.instructions.push_back(VMInstr::JMP(condition));
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions[end_loop] = VMInstr::JMPF(curr_frame.instructions.size() - 1);
  var_table.pop_environment();
}


void CodeGenerator::visit(IfStmt& s)
{ 
  std::vector<int> false_jmps_loc;
  std::vector<int> end_jmp_loc;
  // if
  s.if_part.condition.accept(*this);
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  false_jmps_loc.push_back(curr_frame.instructions.size() - 1); // first at 0
  // stmts
  var_table.push_environment();
  for (int i =0; i < s.if_part.stmts.size(); i++){
    s.if_part.stmts[i] -> accept(*this);
  }
  var_table.pop_environment();

  // for jumping
  curr_frame.instructions.push_back(VMInstr::JMP(-1)); // JMP to end of if stmt 
  end_jmp_loc.push_back(curr_frame.instructions.size() -1);
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions[false_jmps_loc[0]] = VMInstr::JMPF(curr_frame.instructions.size() -1);

  // else ifs
  var_table.push_environment();
  for (int i = 0; i < s.else_ifs.size(); i++){
    // saving index of start of if stmt
    s.else_ifs[i].condition.accept(*this);
    curr_frame.instructions.push_back(VMInstr::JMPF(-1));
    false_jmps_loc.push_back(curr_frame.instructions.size() -1); // second at 1 ...
    // stmts within else ifs
    for (int j = 0; j < s.if_part.stmts.size(); j++){
      s.else_ifs[i].stmts[j] -> accept(*this);
    }

    // for jmping
    curr_frame.instructions.push_back(VMInstr::JMP(-1)); // JMP to end of if stmt 
    end_jmp_loc.push_back(curr_frame.instructions.size() -1);
    curr_frame.instructions.push_back(VMInstr::NOP());
    curr_frame.instructions[false_jmps_loc[i+1]] = VMInstr::JMPF(curr_frame.instructions.size() -1);
  }
  var_table.pop_environment();
  
  // else
  var_table.push_environment();
  false_jmps_loc.push_back(curr_frame.instructions.size());
  for (int i = 0; i < s.else_stmts.size(); i++){
    s.else_stmts[i] -> accept(*this);
  }
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::NOP());
  for (int i = 0; i < end_jmp_loc.size(); i++){
    curr_frame.instructions[end_jmp_loc[i]] = VMInstr::JMP(curr_frame.instructions.size() - 1);
  }
}


void CodeGenerator::visit(VarDeclStmt& s)
{
  var_table.add(s.var_def.var_name.lexeme());
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.var_def.var_name.lexeme())));
}


void CodeGenerator::visit(AssignStmt& s)
{ 
  if (s.lvalue.size() == 1){
    // Arrays
    if(s.lvalue[0].array_expr != nullopt){
      // Pushes oid for array
      curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue[0].var_name.lexeme())));
      // Pushes array index
      s.lvalue[0].array_expr -> accept(*this);
      // Pushes value
      s.expr.accept(*this);
      curr_frame.instructions.push_back(VMInstr::SETI());
    }
    else{
      // pushes value
      s.expr.accept(*this);
      // Stores into variable
      curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.lvalue[0].var_name.lexeme())));
    }
  }
  else{
    // loads initial value
    curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue[0].var_name.lexeme())));
    // setting struct fields
    if (s.lvalue[s.lvalue.size() - 1].array_expr == nullopt){
      for (int i = 1; i < s.lvalue.size() -1; i++){
        if(s.lvalue[i-1].array_expr != nullopt){
          s.lvalue[i-1].array_expr -> accept(*this);
          curr_frame.instructions.push_back(VMInstr::GETI());
        }
        else{
          curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i].var_name.lexeme()));
        }
      }
      if(s.lvalue[s.lvalue.size() - 2].array_expr != nullopt){
        s.lvalue[s.lvalue.size() - 2].array_expr -> accept(*this);
        curr_frame.instructions.push_back(VMInstr::GETI());
      }

      s.expr.accept(*this);
      string name = s.lvalue[s.lvalue.size() - 1].var_name.lexeme();
      curr_frame.instructions.push_back(VMInstr::SETF(name));
    }
    // setting array vals
    else{
      for (int i = 0; i < s.lvalue.size(); i++){
        if(s.lvalue[i].array_expr != nullopt){
          s.lvalue[i].array_expr -> accept(*this);
          if (i != s.lvalue.size() - 1){
            curr_frame.instructions.push_back(VMInstr::GETI());
          }
        }
        else{
          curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i + 1].var_name.lexeme()));
        }
      }
      // value
      s.expr.accept(*this);
      curr_frame.instructions.push_back(VMInstr::SETI());
    }
  }
}


void CodeGenerator::visit(CallExpr& e)
{
  for (int i = 0; i < e.args.size(); i++){
    e.args[i].accept(*this);
  }
  // BUILT INS
  if (e.fun_name.lexeme() == "print"){
    curr_frame.instructions.push_back(VMInstr::WRITE());
  }
  else if (e.fun_name.lexeme() == "to_string"){
    curr_frame.instructions.push_back(VMInstr::TOSTR());
  }
  else if (e.fun_name.lexeme() == "to_int"){
    curr_frame.instructions.push_back(VMInstr::TOINT());
  }
  else if (e.fun_name.lexeme() == "to_double"){
    curr_frame.instructions.push_back(VMInstr::TODBL());
  }
  else if (e.fun_name.lexeme() == "input"){
    curr_frame.instructions.push_back(VMInstr::READ());
  }
  else if (e.fun_name.lexeme() == "concat"){
    curr_frame.instructions.push_back(VMInstr::CONCAT());
  }
  else if (e.fun_name.lexeme() == "length"){
    curr_frame.instructions.push_back(VMInstr::SLEN());
  }
  else if (e.fun_name.lexeme() == "length@array"){
    curr_frame.instructions.push_back(VMInstr::ALEN());
  }
  else if (e.fun_name.lexeme() == "get"){
    curr_frame.instructions.push_back(VMInstr::GETC());
  }
  // LIST FUNCS
  else if (e.fun_name.lexeme() == "list_add"){
    curr_frame.instructions.push_back(VMInstr::DUP());
    curr_frame.instructions.push_back(VMInstr::ADDLI());
    curr_frame.instructions.push_back(VMInstr::SETLE());
  }
  else if (e.fun_name.lexeme() == "list_numi"){
    curr_frame.instructions.push_back(VMInstr::LNUMI());
  }
  else if (e.fun_name.lexeme() == "list_numd"){
    curr_frame.instructions.push_back(VMInstr::LNUMD());
  }
  else if (e.fun_name.lexeme() == "list_nums"){
    curr_frame.instructions.push_back(VMInstr::LNUMS());
  }
  else if (e.fun_name.lexeme() == "list_numb"){
    curr_frame.instructions.push_back(VMInstr::LNUMB());
  }
  else if (e.fun_name.lexeme() == "list_rmb"){
    curr_frame.instructions.push_back(VMInstr::LRMB());
  }
  else if (e.fun_name.lexeme() == "list_avgi"){
    curr_frame.instructions.push_back(VMInstr::LAVGI());
  }
  else if (e.fun_name.lexeme() == "list_avgd"){
    curr_frame.instructions.push_back(VMInstr::LAVGD());
  }
  else if (e.fun_name.lexeme() == "list_change"){
    curr_frame.instructions.push_back(VMInstr::SETLI());
  }
  else if (e.fun_name.lexeme() == "list_size"){
    curr_frame.instructions.push_back(VMInstr::LSIZE());
  }
  else if (e.fun_name.lexeme() == "list_create"){
    curr_frame.instructions.push_back(VMInstr::ALLOCL());
  }
  else if (e.fun_name.lexeme().substr(0, 5) == "list<" && e.fun_name.lexeme().substr((e.fun_name.lexeme().length() - 9), 9) == ">retrieve"){
    curr_frame.instructions.push_back(VMInstr::LRETRIEVE());
  }
  // USER DEFINDED
  else{
    curr_frame.instructions.push_back(VMInstr::CALL(e.fun_name.lexeme()));
  }
} 


void CodeGenerator::visit(Expr& e)
{
  //pushes first onto stack
  e.first -> accept(*this);
  if (e.op != std::nullopt){
    // pushes rest onto stack
    if(e.rest != nullptr){
      e.rest -> accept(*this);
    }
    // push instructions relating to ops type
    string op = e.op -> lexeme();
    if(op == "+"){
      curr_frame.instructions.push_back(VMInstr::ADD());
    }
    else if(op == "-"){
      curr_frame.instructions.push_back(VMInstr::SUB());
    }
    else if(op == "/"){
      curr_frame.instructions.push_back(VMInstr::DIV());
    }
    else if(op == "*"){
      curr_frame.instructions.push_back(VMInstr::MUL());
    }
    else if(op == "=="){
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
    }
    else if(op == "!="){
      curr_frame.instructions.push_back(VMInstr::CMPNE());
    }
    else if(op == "<="){
      curr_frame.instructions.push_back(VMInstr::CMPLE());
    }
    else if(op == ">="){
      curr_frame.instructions.push_back(VMInstr::CMPGE());
    }
    else if(op == ">"){
      curr_frame.instructions.push_back(VMInstr::CMPGT());
    }
    else if(op == "<"){
      curr_frame.instructions.push_back(VMInstr::CMPLT());
    }
    else if(op == "and"){
      curr_frame.instructions.push_back(VMInstr::AND());
    }
    else if(op == "or"){
      curr_frame.instructions.push_back(VMInstr::OR());
    }
  }
  if (e.negated){
    curr_frame.instructions.push_back(VMInstr::NOT());
  }
}


void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue -> accept(*this);
}
 

void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


void CodeGenerator::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL){
    curr_frame.instructions.push_back(VMInstr::PUSH(stoi(v.value.lexeme())));
  }
  else if (v.value.type() == TokenType::DOUBLE_VAL){
    curr_frame.instructions.push_back(VMInstr::PUSH(stod(v.value.lexeme())));  
  }  
  else if (v.value.type() == TokenType::CHAR_VAL){
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));   
  }
  else if (v.value.type() == TokenType::STRING_VAL){
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));    
  }
  else if (v.value.type() == TokenType::BOOL_VAL){
    if (v.value.lexeme() == "true"){
      curr_frame.instructions.push_back(VMInstr::PUSH(true));  
    }
    else if (v.value.lexeme() == "false"){
      curr_frame.instructions.push_back(VMInstr::PUSH(false));  
    }
  }
  else if (v.value.type() == TokenType::NULL_VAL){
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));  
  }
}


void CodeGenerator::visit(NewRValue& v)
{ 
  // STRUCT ALLOCATION
  if (struct_defs.count(v.type.lexeme()) == 1){
    StructDef s = struct_defs[v.type.lexeme()];
    if (v.array_expr == nullopt){
      curr_frame.instructions.push_back(VMInstr::ALLOCS());
      // struct fields
      for (int i = 0; i < s.fields.size(); i++){
        curr_frame.instructions.push_back(VMInstr::DUP());
        curr_frame.instructions.push_back(VMInstr::ADDF(s.fields[i].var_name.lexeme()));
        curr_frame.instructions.push_back(VMInstr::DUP());
        curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
        curr_frame.instructions.push_back(VMInstr::SETF(s.fields[i].var_name.lexeme()));
      }
    }
    else {
      // handles arrays of structs
      v.array_expr -> accept(*this);
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::ALLOCA());
    }
  }
  // Array creation
  else {
    // push length
    v.array_expr -> accept(*this);
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::ALLOCA());
  }
}


void CodeGenerator::visit(VarRValue& v)
{
  VMInstr initial = VMInstr::LOAD(var_table.get(v.path[0].var_name.lexeme()));
  initial.set_comment("VarRVal");
  curr_frame.instructions.push_back(initial);
  for (int i = 0; i < v.path.size(); i++){
    if(v.path[i].array_expr != nullopt){
      if(v.path.size() > 1  && i != 0){
        curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
      }
      v.path[i].array_expr -> accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
    else if (i > 0){
      curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
    }
  }   
}
    

