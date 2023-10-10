//----------------------------------------------------------------------
// FILE: list_testss.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Basic list tests
//----------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include <token.h>
#include <lexer.h>
#include <simple_parser.h>
#include <mypl_exception.h>
#include <ast_parser.h>
#include <print_visitor.h>
#include <ast.h>
#include <semantic_checker.h>
#include <vm.h>
#include <code_generator.h>

using namespace std;


streambuf* stream_buffer;


void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s + "\n";
  return result;
}

//------------------------------------------------------------
// Lexer tests
//------------------------------------------------------------

TEST(ListLexerTests, List_retrieve) {
  stringstream in(build_string({
        "list<int>retrieve",
        "list<bool>retrieve",
        "list<double>retrieve",
        "list<string>retrieve",
        "list<array>retrieve",
        "list<user_defined>retrieve",
      }));
  Lexer lexer(in);
  Token t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<int>retrieve", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<bool>retrieve", t.lexeme());
  ASSERT_EQ(2, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<double>retrieve", t.lexeme());
  ASSERT_EQ(3, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<string>retrieve", t.lexeme());
  ASSERT_EQ(4, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<array>retrieve", t.lexeme());
  ASSERT_EQ(5, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<user_defined>retrieve", t.lexeme());
  ASSERT_EQ(6, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::EOS, t.type());
}   

TEST(ListLexerTests, List_keyword) {
  stringstream in(build_string({
        "list",
        "list<bool>retrieve",
        "list_create"
      }));
  Lexer lexer(in);
  Token t = lexer.next_token();
  ASSERT_EQ(TokenType::LIST, t.type());
  ASSERT_EQ("list", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list<bool>retrieve", t.lexeme());
  ASSERT_EQ(2, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ID, t.type());
  ASSERT_EQ("list_create", t.lexeme());
  ASSERT_EQ(3, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::EOS, t.type());
}  

//----------------------------------------------------------------------
// Parser tests
//----------------------------------------------------------------------
        
TEST(ListParserTests, ListCreation) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()"
        "}"
      }));
  ASTParser(Lexer(in)).parse();
}    

//----------------------------------------------------------------------
// Semantic tests
//----------------------------------------------------------------------
        
TEST(ListSemanticTests, BasicListType) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()"
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_NumI_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_numi(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_NumD_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_numd(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_NumS_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_nums(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_NumB_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_numb(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_rmb_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_rmb(my_list)",
        "  bool y = list_rmb(my_list)",
        "  string z = list_rmb(my_list)",
        "  double b = list_rmb(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_AvgI_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_avgi(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_AvgD_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  double x = list_avgd(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_change_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_change(my_list, 0, 5)",
        "  string b = list_change(my_list, 0, 5)",
        "  bool d = list_change(my_list, 0, 5)",
        "  double z = list_change(my_list, 0, 5)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_size_Type) {
  stringstream in(build_string({
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list_size(my_list)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(ListSemanticTests, List_retrieve_Type) {
  stringstream in(build_string({
        "struct T {int t}",
        "void main() {",
        "  list my_list = list_create()",
        "  int x = list<int>retrieve(my_list, 0)",
        "  double y = list<double>retrieve(my_list, 0)",
        "  bool b = list<bool>retrieve(my_list, 0)",
        "  string s = list<string>retrieve(my_list, 0)",
        "  T F = list<T>retrieve(my_list, 0)",
        "}"
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

//----------------------------------------------------------------------
// VM tests
//----------------------------------------------------------------------
 
TEST(ListVMTests, List_creation) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::WRITE());
  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023", out.str());
  restore_cout();
}

TEST(ListVMTests, List_add_element) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023", out.str());
  restore_cout();
}

TEST(ListVMTests, List_get_set_element) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::GETLI());
  main.instructions.push_back(VMInstr::WRITE());
  // PC 10
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH(2.2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::GETLI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(true));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::GETLI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(3));
  main.instructions.push_back(VMInstr::PUSH("hello"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(3));
  main.instructions.push_back(VMInstr::GETLI());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("202322.200000truehello", out.str());
  restore_cout();
}

TEST(ListVMTests, List_size) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LSIZE());
  main.instructions.push_back(VMInstr::WRITE());
  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("20231", out.str());
  restore_cout();
}

TEST(ListVMTests, List_numi) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(3));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMI());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023112", out.str());
  restore_cout();
}

TEST(ListVMTests, List_numd) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(2.2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMD());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMD());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(3.3));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMD());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023112", out.str());
  restore_cout();
}

TEST(ListVMTests, List_nums) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMS());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMS());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMS());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023112", out.str());
  restore_cout();
}

TEST(ListVMTests, List_numb) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(true));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMB());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMB());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(false));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMB());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023112", out.str());
  restore_cout();
}

TEST(ListVMTests, List_avgi) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LAVGI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LNUMI());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(4));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LAVGI());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("2023112", out.str());
  restore_cout();
}

TEST(ListVMTests, List_avgd) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(1.5));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LAVGD());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(4.3));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LAVGD());
  main.instructions.push_back(VMInstr::WRITE());

  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("20231.5000002.900000", out.str());
  restore_cout();
}

TEST(ListVMTests, List_rmb) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LSIZE());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LSIZE());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LRMB());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LSIZE());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH(3));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::LSIZE());
  main.instructions.push_back(VMInstr::WRITE());
  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("20231212", out.str());
  restore_cout();
}

TEST(ListVMTests, List_retrieve) {
  VMFrameInfo main {"main", 0};
  main.instructions.push_back(VMInstr::ALLOCL());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::PUSH(1.5));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(0));
  main.instructions.push_back(VMInstr::LRETRIEVE());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::PUSH("abc"));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(1));
  main.instructions.push_back(VMInstr::LRETRIEVE());
  main.instructions.push_back(VMInstr::WRITE());

  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::ADDLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::PUSH(4.3));
  main.instructions.push_back(VMInstr::SETLI());
  main.instructions.push_back(VMInstr::PUSH(2023));
  main.instructions.push_back(VMInstr::PUSH(2));
  main.instructions.push_back(VMInstr::LRETRIEVE());
  main.instructions.push_back(VMInstr::WRITE());


  VM vm; 
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("20231.500000abc4.300000", out.str());
  restore_cout();
}


//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

