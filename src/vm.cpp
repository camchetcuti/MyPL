//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: VM Operations for mypl
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"


using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}


void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}


void VM::run(bool DEBUG)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);

  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];

    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }

    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }

    else if (instr.opcode() == OpCode::LOAD){
      int var_location = get<int>(instr.operand().value());
      frame->operand_stack.push(frame->variables[var_location]);
    }

    else if (instr.opcode() == OpCode::STORE){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      int var_location = get<int>(instr.operand().value());
      if(var_location >= frame->variables.size()){
        frame->variables.push_back(x);
      }
      else{
        frame->variables[var_location] = x;
      }
    }

    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    else if (instr.opcode() == OpCode::SUB){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }

    else if (instr.opcode() == OpCode::MUL){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }

    else if (instr.opcode() == OpCode::DIV){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }

    else if (instr.opcode() == OpCode::AND){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame -> operand_stack.push(get<bool>(x) && get<bool>(y));
    }

    else if (instr.opcode() == OpCode::OR){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame -> operand_stack.push(get<bool>(x) || get<bool>(y));
    }

    else if (instr.opcode() == OpCode::NOT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame -> operand_stack.push(!get<bool>(x));
    }

    else if (instr.opcode() == OpCode::CMPLT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }

    else if (instr.opcode() == OpCode::CMPLE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }

    else if (instr.opcode() == OpCode::CMPGT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }
    
    else if (instr.opcode() == OpCode::CMPGE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }

    else if (instr.opcode() == OpCode::CMPEQ){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y, x));
    }

    else if (instr.opcode() == OpCode::CMPNE){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(ne(y, x));
    }

    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::JMP){
      frame->pc = get<int>(instr.operand().value());
    }
    
    else if (instr.opcode() == OpCode::JMPF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if (!get<bool>(x)){
        frame->pc = get<int>(instr.operand().value());
      }
    }

    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::CALL){
      string fun_name = get<string>(instr.operand().value());
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      new_frame->info = frame_info[fun_name]; 
      call_stack.push(new_frame);
      for (int i = 0; i < new_frame->info.arg_count; i++){
        VMValue x = frame -> operand_stack.top();
        new_frame->operand_stack.push(x);
        frame->operand_stack.pop();
      }
      frame = new_frame;
    }

    else if (instr.opcode() == OpCode::RET){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      call_stack.pop();
      if(call_stack.size() != 0){
        frame = call_stack.top();
        if(frame != nullptr){
          frame->operand_stack.push(x);
        }
      }
    }
    
    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }
    
    else if (instr.opcode() == OpCode::SLEN){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int len = to_string(x).length();
      frame -> operand_stack.push(len);
    }

    else if (instr.opcode() == OpCode::ALEN){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int oid = get<int>(x);
      vector<VMValue> array_for_len = array_heap.at(oid);
      int len = array_for_len.size();
      frame -> operand_stack.push(len);
    }

    else if (instr.opcode() == OpCode::GETC){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      if (stoi(to_string(y)) > to_string(x).length() -1 || stoi(to_string(y)) < 0){
        error("out-of-bounds string index", *frame);
      }
      string xstr = to_string(x);
      int yint = get<int>(y);
      string c = "";
      c.push_back(xstr[yint]);
      frame -> operand_stack.push(c);
    }

    else if (instr.opcode() == OpCode::TOINT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if (holds_alternative<int>(x)) {
        frame -> operand_stack.push(to_string(x));
      }
      else if (holds_alternative<string>(x)){
        try{
          frame -> operand_stack.push(stoi(to_string(x)));
        }
        catch (exception& e) {
          error("cannot convert string to int", *frame);
        }
      }
      else if (holds_alternative<double>(x)){
        frame -> operand_stack.push(stoi(to_string(x)));
      }
      else if (holds_alternative<nullptr_t>(x)){
        frame -> operand_stack.push(nullptr);
      }
    }

    else if (instr.opcode() == OpCode::TODBL){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if (holds_alternative<int>(x)) {
        frame -> operand_stack.push(stod(to_string(x)));
      }
      else if (holds_alternative<string>(x)){
        try{
          frame -> operand_stack.push(stod(to_string(x)));
        }
        catch (exception& e) {
          error("cannot convert string to double", *frame);
        }
      }
      else if (holds_alternative<nullptr_t>(x)){
        frame -> operand_stack.push(nullptr);
      }
    }

    else if (instr.opcode() == OpCode::TOSTR){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(to_string(x));
    }

    else if (instr.opcode() == OpCode::CONCAT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(to_string(y) + to_string(x));
    }
    
    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ALLOCS){
      std::unordered_map<std::string, VMValue> new_struct;
      struct_heap.emplace(next_obj_id, new_struct);
      frame->operand_stack.push(next_obj_id);
      next_obj_id = next_obj_id + 1; 
    }

    else if (instr.opcode() == OpCode::ALLOCA){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      vector<VMValue> new_array (get<int>(y), x);
      array_heap.emplace(next_obj_id, new_array);
      frame->operand_stack.push(next_obj_id);
      next_obj_id = next_obj_id + 1;  
    }

    // LISTS
    else if (instr.opcode() == OpCode::ALLOCL){
      std::unordered_map<int, VMValue> new_list;
      list_heap.emplace(next_obj_id, new_list);
      frame->operand_stack.push(next_obj_id);
      next_obj_id = next_obj_id + 1;
    }

    else if (instr.opcode() == OpCode::ADDLI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      list_heap.at(get<int>(x)).emplace(list_heap.at(get<int>(x)).size(), nullptr);   
    }

    else if (instr.opcode() == OpCode::SETLE){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      list_heap.at(get<int>(x))[list_heap.at(get<int>(x)).size() - 1] = y;
    }

    else if (instr.opcode() == OpCode::SETLI){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();
      list_heap.at(get<int>(z))[get<int>(y)] = x;
    }

    else if (instr.opcode() == OpCode::GETLI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      VMValue value = list_heap.at(get<int>(y)).at(get<int>(x));
      if (holds_alternative<int>(value)){
        frame->operand_stack.push(get<int>(value));
      }
      else if (holds_alternative<bool>(value)){
        frame->operand_stack.push(get<bool>(value));
      }
      else if (holds_alternative<string>(value)){
        frame->operand_stack.push(get<string>(value));
      }
      else if (holds_alternative<double>(value)){
        frame->operand_stack.push(get<double>(value));
      }
      else {
        frame->operand_stack.push(get<nullptr_t>(value));
      }
    }

    else if (instr.opcode() == OpCode::LNUMI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<int>(value)){
          num++;
        }
      }
      frame->operand_stack.push(num);
    }

    else if (instr.opcode() == OpCode::LNUMD){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<double>(value)){
          num++;
        }
      }
      frame->operand_stack.push(num);
    }

    else if (instr.opcode() == OpCode::LNUMS){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<string>(value)){
          num++;
        }
      }
      frame->operand_stack.push(num);
    }

    else if (instr.opcode() == OpCode::LNUMB){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<bool>(value)){
          num++;
        }
      }
      frame->operand_stack.push(num);
    }

    else if (instr.opcode() == OpCode::LRMB){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int size = list_heap.at(get<int>(x)).size();
      list_heap.at(get<int>(x)).erase(size - 1);
    }
    
    else if (instr.opcode() == OpCode::LAVGI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      int sum = 0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<int>(value)){
          sum += get<int>(value);
          num++;
        }
      }
      if (num != 0){
        frame->operand_stack.push(sum / num);
      }
      else {
        frame->operand_stack.push(0);
      }
    }

    else if (instr.opcode() == OpCode::LAVGD){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int num = 0;
      double sum = 0.0;
      int size = list_heap.at(get<int>(x)).size();
      VMValue value;
      for (int i = 0; i < size; i++){
        value = list_heap.at(get<int>(x)).at(i);
        if (holds_alternative<double>(value)){
          sum += get<double>(value);
          num++;
        }
      }
      if (num != 0){
        double avg = sum / double(num);
        frame->operand_stack.push(avg);
      }
      else {
        frame->operand_stack.push(0.0);
      }
    }

    else if (instr.opcode() == OpCode::LSIZE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int size = list_heap.at(get<int>(x)).size();
      frame->operand_stack.push(size);
    }

    else if (instr.opcode() == OpCode::LRETRIEVE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      int size = list_heap.at(get<int>(y)).size();
      VMValue value;
      if (get<int>(x) > size - 1){
        value = list_heap.at(get<int>(y)).at(size - 1);
      }
      else{
        value = list_heap.at(get<int>(y)).at(get<int>(x));
      }
      //retrieval
      if (holds_alternative<int>(value)){
        frame->operand_stack.push(get<int>(value));
      }
      else if (holds_alternative<string>(value)){
        frame->operand_stack.push(get<string>(value));
      }
      else if (holds_alternative<double>(value)){
        frame->operand_stack.push(get<double>(value));
      }
      else if (holds_alternative<bool>(value)){
        frame->operand_stack.push(get<bool>(value));
      }
    }
    //LISTS

    else if (instr.opcode() == OpCode::ADDF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      struct_heap.at(get<int>(x)).emplace(get<string>(instr.operand().value()), nullptr);
    }

    else if (instr.opcode() == OpCode::SETF){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      struct_heap.at(get<int>(y)).at(get<string>(instr.operand().value())) = x;
    }

    else if (instr.opcode() == OpCode::GETF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(struct_heap.at(get<int>(x)).at(get<string>(instr.operand().value())));
    }

    else if (instr.opcode() == OpCode::SETI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();
      if(get<int>(y) >= array_heap.at(get<int>(z)).size() || get<int>(y) < 0){
        error("out-of-bounds array index", *frame);
      }
      array_heap.at(get<int>(z))[get<int>(y)] = x;
    }

    else if (instr.opcode() == OpCode::GETI){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      if(get<int>(x) >= array_heap.at(get<int>(y)).size() || get<int>(x) < 0){
        error("out-of-bounds array index", *frame);
      }
      frame->operand_stack.push(array_heap.at(get<int>(y))[get<int>(x)]);
    }
    
    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
}


void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}

VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}

VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) - get<int>(y);
  }
  else{
    return get<double>(x) - get<double>(y);
  }
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) * get<int>(y);
  }
  else{
    return get<double>(x) * get<double>(y);
  }
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) / get<int>(y);
  }
  else{
    return get<double>(x) / get<double>(y);
  }
}

VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x)){
    return get<string>(x) == get<string>(y);
  }
  else
    return get<bool>(x) == get<bool>(y);
}

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) < get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) < get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) < get<string>(y);
  else
    return get<bool>(x) < get<bool>(y);
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true; 
  else if (holds_alternative<int>(x)) 
    return get<int>(x) <= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) <= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) <= get<string>(y);
  else
    return get<bool>(x) <= get<bool>(y);
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true; 
  else if (holds_alternative<int>(x)) 
    return get<int>(x) > get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) > get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) > get<string>(y);
  else
    return get<bool>(x) > get<bool>(y);  
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true; 
  else if (holds_alternative<int>(x)) 
    return get<int>(x) >= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) >= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) >= get<string>(y);
  else
    return get<bool>(x) >= get<bool>(y);
}

VMValue VM::ne(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return true;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) != get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) != get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x).compare(get<string>(y)) != 0;
  else
    return get<bool>(x) != get<bool>(y);
}

