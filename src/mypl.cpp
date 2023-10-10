//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: Spring 2023
// AUTH: Cameron Chetcuti
// DESC: Basic functions for MyPL assignment 1.
//----------------------------------------------------------------------

#include <iostream>
#include <fstream>
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

void printHelpMenu();
bool checkFileName(string);

int main(int argc, char* argv[])
{
  // checking for correct number of arguments
int x = argc;
  if (x > 3){
    printHelpMenu();
  }
  // NORMAL
  if (x == 1){
    cout << "[Normal Mode]" << endl;
    try {
      Lexer lexer = Lexer(cin);
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker t;
      p.accept(t);
      VM vm;
      CodeGenerator g(vm);
      p.accept(g);
      vm.run();
    } catch (MyPLException& ex){
      cerr << ex.what() << endl;
    }
  }
  bool inputSelected = false;
  if (x > 1){
    string option(argv[1]);
    
    // HELP
    if (option == "--help"){
      printHelpMenu();
      inputSelected = true;
    }
    // LEXER
    else if (option == "--lex"){
      cout << "[Lex Mode]" << endl;
      if (x == 2){
        Lexer lexer = Lexer(cin);

        // ASKS FOR INPUT FROM CMD LINE HERE
        try {
          Token t = lexer.next_token();
          cout << to_string(t) << endl;
          while (t.type() != TokenType::EOS){
            t = lexer.next_token();
            cout << to_string(t) << endl;
          }
        } catch (MyPLException& ex){
          cerr << ex.what() << endl;
        }
      }
      // IF THE USER CHOOSES TO INTERACT WITH A FILE
      // THAT IS DEALT WITH HERE
      if (x > 2){
        string filename(argv[2]);
        // OPENING INPUT FILENAME
        ifstream inFile(filename);
        if (inFile){
          Lexer lexer = Lexer(inFile);
          // READ IN FROM FILE
          try {
            Token t = lexer.next_token();
            cout << to_string(t) << endl;
            while (t.type() != TokenType::EOS){
              t = lexer.next_token();
              cout << to_string(t) << endl;
            }
          }  
          catch (MyPLException& ex){
            cerr << ex.what() << endl;
          }
        }
      }
    }
    //PARSER
    else if (option == "--parse"){
      cout << "[Parse Mode]" << endl;
      if (x == 2){
        // // ASKS FOR INPUT FROM CMD LINE HERE
        try {
          Lexer lexer = Lexer(cin);
          ASTParser parser(lexer);
          parser.parse();
        }
        catch (MyPLException& ex){
          cerr << ex.what() << endl;
        }
      }

      // IF THE USER CHOOSES TO INTERACT WITH A FILE
      // THAT IS DEALT WITH HERE
      if (x > 2){
        string output = "";
        string filename(argv[2]);
        // OPENING INPUT FILENAME
        ifstream inFile(filename);
        if (inFile){
          try {
            Lexer lexer = Lexer(inFile);
            ASTParser parser(lexer);
            parser.parse();
          }
          catch (MyPLException& ex){
            cerr << ex.what() << endl;
          }
        }
      }
    }
    
    // PRINTER
    else if (option == "--print"){
      if (x == 2){
        try{
          Lexer lexer = Lexer(cin);
          ASTParser parser(lexer);
          Program p = parser.parse();
          PrintVisitor v(cout);
          p.accept(v);
        } catch (MyPLException& ex){
          cerr << ex.what() << endl;
        }
      }
      // IF THE USER CHOOSES TO INTERACT WITH A FILE
      // THAT IS DEALT WITH HERE
      if (x > 2){
          string output = "";
          string filename(argv[2]);
          // OPENING INPUT FILENAME
          ifstream inFile(filename);
          if (inFile){
            try{
              Lexer lexer = Lexer(inFile);
              ASTParser parser(lexer);
              Program p = parser.parse();
              PrintVisitor v(cout);
              p.accept(v);
            } catch (MyPLException& ex){
              cerr << ex.what() << endl;
            }
          }
        }
    }
    // CHECK
    else if (option == "--check"){
      cout << "[Check Mode]" << endl;
      if (x == 2){
        try{
          Lexer lexer = Lexer(cin);
          ASTParser parser(lexer);
          Program p = parser.parse();
          SemanticChecker v;
          p.accept(v);
        } catch (MyPLException& ex) {
          cerr << ex.what() << endl;
        }
      }
      // IF THE USER CHOOSES TO INTERACT WITH A FILE
      // THAT IS DEALT WITH HERE
      if (x > 2){
        string output = "";
        string filename(argv[2]);
        // OPENING INPUT FILENAME
        ifstream inFile(filename);
        if (inFile){
          try{
            Lexer lexer = Lexer(inFile);
            ASTParser parser(lexer);
            Program p = parser.parse();
            SemanticChecker v;
            p.accept(v);
          } catch (MyPLException& ex) {
            cerr << ex.what() << endl;
          }
        }
      }
    }
    // IR
    else if (option == "--ir"){
      cout << "[IR Mode]" << endl;
      if (x == 2){
        // ASKS FOR INPUT FROM CMD LINE HERE
        try {
          Lexer lexer = Lexer(cin);
          ASTParser parser(lexer);
          Program p = parser.parse();
          SemanticChecker t;
          p.accept(t);
          VM vm;
          CodeGenerator g(vm);
          p.accept(g);
          cout << to_string(vm) << endl;
        } catch (MyPLException& ex){
          cerr << ex.what() << endl;
        }
      }
      // IF THE USER CHOOSES TO INTERACT WITH A FILE
      // THAT IS DEALT WITH HERE
      if (x > 2){
        string output = "";
        string filename(argv[2]);
        // OPENING INPUT FILENAME
        ifstream inFile(filename);
        if (inFile){
          // READ IN FROM FILE
          try {
            Lexer lexer = Lexer(inFile);
            ASTParser parser(lexer);
            Program p = parser.parse();
            SemanticChecker t;
            p.accept(t);
            VM vm;
            CodeGenerator g(vm);
            p.accept(g);
            cout << to_string(vm) << endl;
          } catch (MyPLException& ex){
            cerr << ex.what() << endl;
          } 
        } 
        cout << output << endl;
      }
    }
    // NORMAL 
    else{ 
      string output = "";
      if (x > 1){
        string filename(argv[1]);
        // OPENING INPUT FILENAME
        ifstream inFile(filename);
        if (inFile){
          cout << "[Normal Mode]" << endl;
          // READ IN FROM CMD LINE
           try {
            Lexer lexer = Lexer(inFile);
            ASTParser parser(lexer);
            Program p = parser.parse();
            SemanticChecker t;
            p.accept(t);
            VM vm;
            CodeGenerator g(vm);
            p.accept(g);
            vm.run();
          } catch (MyPLException& ex){
            cerr << ex.what() << endl;
          } 
        }
        // INVALID
        else {
          cout << "ERROR: Unable to open file '" << option << "'" << endl;
        }
      } 
    }   
  }
}
  


/*
  Function prints the help menu message with correct formatting.
*/
void printHelpMenu(){
  cout << "Usage: ./mypl [option] [script-file]" << endl;
  cout << "Options:" << endl;
  cout << " --help      prints this message" << endl;
  cout << " --lex       displays token information" << endl;
  cout << " --parse     checks for syntax errors" << endl;
  cout << " --print     pretty prints program" << endl;
  cout << " --check     statically checks program" << endl;
  cout << " --ir        print intermediate (code) representation" << endl;
}

