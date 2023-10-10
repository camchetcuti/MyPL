//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME: Cameron Chetcuti
// DESC: A program which provides lexical analysis for myPl
//----------------------------------------------------------------------

#include "lexer.h"

#include <iostream>

using namespace std;


Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}


char Lexer::read()
{
  ++column;
  return input.get();
}


char Lexer::peek()
{
  return input.peek();
}


void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}


Token Lexer::next_token()
{
  // Stores first characters value
  char first_char;
  // Variable for the current character
  char input_char;
  // Variable for the next character
  char next_char = ' ';
  // Creates the Token for this instance
  Token token = Token();
  // String of previous characters in token lexeme
  string token_lex = "";
  // counter for \n token length checking
  int counter = 0;
  // Flag to see if all the input is numeric
  bool is_all_digit = true;
  // Serves as a flag to see if a double is to be output
  bool double_flag = false;
  // FLAG FOR LIST_RETRIEVE
  bool is_list_retrieve = false;
  // Checks for invalid not notation
  string invalid_not = "";
  // This controls if an id is to be output
  bool endID = false;
  // Array for checking invalid id characters
  string tokenArray = "\"\'.;,()[]{}#+=-*/<>! \n";

  // This loop goes while reading in characters
  while (next_char != EOF){

    input_char = read();
    next_char = peek();

    switch (input_char){
    
      // STRINGS

      case '"':
        if (token_lex == ""){
          token_lex.push_back('"');
          first_char = '"';
          if(next_char == EOF){
            input_char = read();
            next_char = peek();
            error("found end-of-file in string", line, column);
          }
        }
        else if ((token_lex != "") && (token_lex[0] == '"')){
          token_lex.push_back('"');
          string stringVal;
          for (int i = 0; i < token_lex.length(); i++){
            if (token_lex[i] != '\"'){
              stringVal.push_back(token_lex[i]);
            }
          }
          char empty = 'e';
          first_char = empty;
          return Token(TokenType::STRING_VAL, stringVal, line, column - stringVal.length() - 1);
        }

      // CHARS
        
      case '\'':
        if (token_lex == ""){
          token_lex.push_back('\'');
          if(next_char == EOF){
            input_char = read();
            next_char = peek();
            error("found end-of-file in character", line, column);
          }
        }
        else if ((token_lex != "") && (token_lex[0] == '\'')){
          token_lex.push_back('\'');
          string charLexeme = "";
          if (token_lex[1] == '\\'){
            charLexeme.push_back(token_lex[1]);
            charLexeme.push_back(token_lex[2]);  
            if (token_lex.length() == 5){
              charLexeme.push_back(token_lex[3]);
            }
          }
          else if(token_lex[1] != '\\' && token_lex.length() > 3){
            string error_str;
            error_str.push_back(token_lex[2]);
            error("expecting \' found " + error_str, line, column -1);
          }
          else{
            charLexeme.push_back(token_lex[1]);
          }
          return Token(TokenType::CHAR_VAL, charLexeme, line, column - charLexeme.length() -1);
        }
      case ' ':
        if ((next_char == '\'') && (input_char == '\'')){
          error("empty character", line, column + 1);
        }
        if (token_lex[0] == '"' && input_char == ' '){
          token_lex.push_back(' ');
        }
        if (token_lex[0] == '\'' && input_char == ' '){
          token_lex.push_back(' ');
        }
        else if (token.type() != TokenType::EOS){
          return Token(TokenType::ID, token_lex, line, column);
        }
        
        break;
      case '\n':
        if (token_lex != ""){
          if(token_lex[0] == '\''){
            error("found end-of-line in character", line, column);
          }
          if(token_lex[0] == '"'){
            error("found end-of-line in string", line, column);
          }
          for (int i = 0; i < token_lex.length(); i++){
            if (!isdigit(token_lex[i])){
              break;
            }
            else {
              counter++;
            }
          }
          if (counter == token_lex.length()){
            return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length());
          }
        }
        line = line + 1;
        column = 0;
        break;

      // PUNCTUATION

      case ';':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::SEMICOLON, ";", line, column);
      case '?':
        if (token_lex == ""){
          error("unexpected character '?'", line, column);
        }
      case '.':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        if (token_lex.length() > 0 && is_all_digit){
          token_lex.push_back(input_char);
          double_flag = true;
          break;
        }
        return Token(TokenType::DOT, ".", line, column); 
      case ',':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::COMMA, ",", line, column);
      case '(':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::LPAREN, "(", line, column);
      case ')':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::RPAREN, ")", line, column);
      case '[':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::LBRACKET, "[", line, column);
      case ']':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::RBRACKET, "]", line, column);
      case '{':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::LBRACE, "{", line, column);
      case '}':
        if(token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::RBRACE, "}", line, column);
      case '#':
        while (next_char != '\n' && next_char != EOF){
          input_char = read();
          next_char = peek();
        }
        if(next_char == EOF){
          column = column + 1;
        }
        break;

      // OPERATORS

      case '+':
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::PLUS, "+", line, column);

      case '-':
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::MINUS, "-", line, column);

      case '*':
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::TIMES, "*", line, column);
      case '/':
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::DIVIDE, "/", line, column);
      case '=':
        // equal
        if (next_char == '='){
          input_char = read();
          next_char = peek();
          if(token_lex[0] != '\"'){
            return Token(TokenType::EQUAL, "==", line, column - 1);
          }
          else {
            token_lex.push_back('=');
            token_lex.push_back('=');            
            break;
          }
        }
        if (token_lex == "'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '"'){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::ASSIGN, "=", line, column);

      
      // COMPARATORS

      case '<':
        if (next_char == '='){
          input_char = read();
          next_char = peek();
          if(token_lex[0] != '\"'){
            return Token(TokenType::LESS_EQ, "<=", line, column - 1);
          }
          else {
            token_lex.push_back('<');
            token_lex.push_back('=');            
            break;
          }
        }
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        // List param funcs
        if (token_lex == "list"){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::LESS, "<", line, column);
      case '>':
        if (next_char == '='){
          input_char = read();
          next_char = peek();
          if(token_lex[0] != '\"'){
            return Token(TokenType::GREATER_EQ, ">=", line, column - 1);
          }
          else {
            token_lex.push_back('>');
            token_lex.push_back('=');            
            break;
          }
        }
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        // List param funcs
        if ((token_lex.substr(0, 5) == "list<")){
          token_lex.push_back(input_char);
          break;
        }
        return Token(TokenType::GREATER, ">", line, column);
        
      case '!':
        if (next_char == '=' && first_char != '"'){
          input_char = read();
          next_char = peek();
          return Token(TokenType::NOT_EQUAL, "!=", line, column - 1);
        }
        if (token_lex == "\'"){
          token_lex.push_back(input_char);
          break;
        }
        if (first_char == '\"'){
          token_lex.push_back(input_char);
          break;
        }
        invalid_not.push_back('!');
        invalid_not.push_back(next_char);
        input_char = read();
        next_char = peek();
        error("expecting '!=' found \'" + invalid_not + "\'", line, column - 1 - token_lex.length());

      // INTS / DOUBLES
      
      case '1':
        if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
      case '2':
        if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
        
      case '3':
        if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
      case '4':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
        
      case '5':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
      case '6':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
      case '7':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }

      case '8':
        if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }

      case '9':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0'){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }

      case '0':
         if (is_all_digit && (next_char == '.') && !double_flag){
          token_lex.push_back(input_char);
          break;
        }
        if (is_all_digit && !isdigit(next_char)){
          token_lex.push_back(input_char);
          if (double_flag){
            return Token(TokenType::DOUBLE_VAL, token_lex, line, column - token_lex.length() + 1);
          }
          first_char = token_lex[0];
          if (token_lex == "'"){
            break;
          }
          if (first_char == '"'){
            break;
          }
          if (first_char == '0' && token_lex.length() > 1){
            error("leading zero in number", line, column - 1);
          }
          return Token(TokenType::INT_VAL, token_lex, line, column - token_lex.length() + 1);
        }
      
      // TOKENS WITH LEXEME > 2

      default : 
        // IF THE VALUE IS A LETTER OR NUMBER
        if (is_all_digit && !isdigit(input_char)){
          is_all_digit = false;
        } 

        if (double_flag && !isdigit(input_char)){
          error("missing digit in \'" + token_lex + "\'", line, column);
        }

        token_lex.push_back(input_char);
        
        
        // RESERVED WORDS

        if (token_lex == "struct"){
          return Token(TokenType::STRUCT, token_lex, line, (column + 1 - token_lex.length()));
        }
        else if (token_lex == "list" && (next_char != '_' && next_char != '<')){
          return Token(TokenType::LIST, token_lex, line, (column + 1 - token_lex.length()));
        }
        else if (token_lex == "for" && (!isalpha(next_char))){
          return Token(TokenType::FOR, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "if" && (!isalpha(next_char))){
          return Token(TokenType::IF, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "else" && (!isalpha(next_char))){
          return Token(TokenType::ELSE, token_lex, line, column + 1 - token_lex.length());
        }  
        else if (token_lex == "while" && (!isalpha(next_char))){
           return Token(TokenType::WHILE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "return" && (!isalpha(next_char))){          
          return Token(TokenType::RETURN, token_lex, line, column + 1 - token_lex.length());
        }  
        else if (token_lex == "new" && (!isalpha(next_char))){
            return Token(TokenType::NEW, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "array" && (!isalpha(next_char))){
            return Token(TokenType::ARRAY, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "not" && (!isalpha(next_char))){
            return Token(TokenType::NOT, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "elseif" && (!isalpha(next_char))){
            return Token(TokenType::ELSEIF, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "or" && (!isalpha(next_char))){
          return Token(TokenType::OR, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "and" && (!isalpha(next_char))){
          return Token(TokenType::AND, token_lex, line, column + 1 - token_lex.length());
        }

        // PRIMITIVE DATA TYPES

        else if (token_lex == "int" && (!isalpha(next_char))){
          return Token(TokenType::INT_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "double" && (!isalpha(next_char))){
          return Token(TokenType::DOUBLE_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "bool" && (!isalpha(next_char))){
          return Token(TokenType::BOOL_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "string" && (!isalpha(next_char))){
          return Token(TokenType::STRING_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "char" && (!isalpha(next_char))){
          return Token(TokenType::CHAR_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "void" && (!isalpha(next_char))){
          return Token(TokenType::VOID_TYPE, token_lex, line, column + 1 - token_lex.length());
        }
        

        // VALUES

        else if (token_lex == "null" && (!isalpha(next_char))){
          return Token(TokenType::NULL_VAL, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "true" && (!isalpha(next_char))){
          return Token(TokenType::BOOL_VAL, token_lex, line, column + 1 - token_lex.length());
        }
        else if (token_lex == "false" && (!isalpha(next_char))){
          return Token(TokenType::BOOL_VAL, token_lex, line, column + 1 - token_lex.length());
        }
        
        // IDENTIFIERS
        else if (isalpha(token_lex[0])){
          for (int i = 0; i < tokenArray.length(); i++){
            if((token_lex.substr(0, 4) == "list") && (token_lex.find("retrieve") != std::string::npos)){
              endID = true; 
            }
            // LIST Funcs
            else if (token_lex == "list_create" || token_lex == "list_numi" || token_lex == "list_numd" ||
                      token_lex == "list_nums" || token_lex == "list_numb" || token_lex == "list_add" || 
                      token_lex == "list_rmb" || token_lex == "list_avgi" || token_lex == "list_avgd" ||
                      token_lex == "list_change" || token_lex == "list_size"
                      && next_char == tokenArray[i]){
              endID = true;
            }
            else if (((next_char == tokenArray[i]) && (token_lex.substr(0, 4) != "list")) || next_char == EOF ){
              endID = true;
            }
          }
        }
        if (endID){
          return Token(TokenType::ID, token_lex, line, column + 1 -  token_lex.length());
        }   
    }
  }
    return Token(TokenType::EOS, "", line, column);
}
  

