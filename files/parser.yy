%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%locations


%code requires{
   #include "ast.hh"
   #include "location.hh"
   #include "other_classes.hh"
   namespace IPL {
      class Scanner;
   }

  // # ifndef YY_NULLPTR
  // #  if defined __cplusplus && 201103L <= __cplusplus
  // #   define YY_NULLPTR nullptr
  // #  else
  // #   define YY_NULLPTR 0
  // #  endif
  // # endif

}

%printer { std::cerr << $$; } STRUCT        
%printer { std::cerr << $$; } MAIN        
%printer { std::cerr << $$; } PRINTF        
%printer { std::cerr << $$; } VOID          
%printer { std::cerr << $$; } INT           
%printer { std::cerr << $$; } FLOAT         
%printer { std::cerr << $$; } LE_OP         
%printer { std::cerr << $$; } GE_OP         
%printer { std::cerr << $$; } RETURN        
%printer { std::cerr << $$; } OR_OP         
%printer { std::cerr << $$; } AND_OP        
%printer { std::cerr << $$; } EQ_OP         
%printer { std::cerr << $$; } NE_OP         
%printer { std::cerr << $$; } IF            
%printer { std::cerr << $$; } ELSE          
%printer { std::cerr << $$; } INC_OP        
%printer { std::cerr << $$; } PTR_OP        
%printer { std::cerr << $$; } WHILE         
%printer { std::cerr << $$; } FOR           
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } INT_CONSTANT  
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } IDENTIFIER    
%printer { std::cerr << $$; } OTHERS    


%parse-param { Scanner  &scanner  }
%locations
%code{
   #include "scanner.hh"
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   #include <map>
   #include <queue>
   #include <vector>

   extern SymbTab gst;
   extern map<string,map<string,int>> LocalStrings;
   int LC_Count=0;
   int hasReturn=0;
   extern std::map<std::string, std::string> predefined;
   LocalSymbTab* runningST, *temprunningST;
   type_specifier_class *running_fun_type,*running_local_type;
   int running_offset, struct_running = 0;
   bool multilevelstruct = false;
   std::map<std::string,abstract_astnode*> ast;
   std::string funcname;
   EntryGlobal* newEntryG;
   EntryLocal* newEntryL;
   std::queue<int> param_sizes;

#undef yylex
#define yylex IPL::Parser::scanner.yylex

}




%define api.value.type variant
%define parse.assert

%start program



%token <std::string> STRUCT                 
%token <std::string> MAIN                 
%token <std::string> PRINTF                 
%token <std::string> VOID                
%token <std::string> INT                 
%token <std::string> FLOAT               
%token <std::string> LE_OP                  
%token <std::string> GE_OP                  
%token <std::string> RETURN                
%token <std::string> OR_OP               
%token <std::string> AND_OP               
%token <std::string> EQ_OP               
%token <std::string> NE_OP              
%token <std::string> IF                
%token <std::string> ELSE                 
%token <std::string> INC_OP           
%token <std::string> PTR_OP            
%token <std::string> WHILE             
%token <std::string> FOR            
%token <std::string> STRING_LITERAL  
%token <std::string> INT_CONSTANT      
%token <std::string> FLOAT_CONSTANT   
%token <std::string> IDENTIFIER        
%token <std::string> OTHERS             
%token ',' '(' ')' ';' '=' '<' '>' '.' '!' '&' '{' '}' '[' ']' '+' '-' '*' '/'

%nterm <abstract_astnode*> compound_statement program translation_unit struct_specifier function_definition main_definition 
%nterm <type_specifier_class*> type_specifier
%nterm <fun_declarator_class*> fun_declarator 
%nterm <parameter_list_class*> parameter_list
%nterm <parameter_declaration_class*> parameter_declaration 
%nterm <declarator_class*> declarator_arr declarator 
%nterm <declaration_class*> declaration 
%nterm <declaration_list_class*> declaration_list
%nterm <declarator_list_class*> declarator_list
%nterm <seq_astnode*> statement_list 
%nterm <assignE_astnode*> assignment_expression
%nterm <assignS_astnode*> assignment_statement
%nterm <exp_astnode*> expression logical_and_expression equality_expression relational_expression additive_expression unary_expression multiplicative_expression postfix_expression primary_expression
%nterm <statement_astnode*> selection_statement
%nterm <statement_astnode*> iteration_statement
%nterm <funcall_astnode*> expression_list
%nterm <proccall_astnode*> procedure_call printf_call
%nterm <std::string> unary_operator
%nterm <statement_astnode*> statement
%%


primary_expression: 
    IDENTIFIER
    {
         $$ = new identifier_astnode($1);
         const EntryLocal* received_entry = runningST->getEntry($1);
         if(received_entry==nullptr){
              error(@$,"Variable \""+ $1 + "\" not declared");
         }
         $$->type = received_entry->type_c;
         $$->label = 1;
    }
    | INT_CONSTANT
    {
         $$ = new intconst_astnode($1);
         std::queue<int> temp;
         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
         $$->type = type_c;
         if($1=="0") $$->type->isZero=1;
         $$->label = 1;
    }
    | '(' expression ')'
    {
         $$ = $2;
    }     
  ;

expression: 
    logical_and_expression
    {
         $$ = $1; 
    }
    | expression OR_OP logical_and_expression
    {
         $$ = new op_binary_astnode("OR_OP", $1, $3); 
         if(($1->type->isStruct==1 && ($1->type->deref_level+$1->type->array_level)==0)||($3->type->isStruct==1 && ($3->type->deref_level+$3->type->array_level)==0)){
              error(@$,"Invalid operand of ||, not scalar or pointer");
         }
         std::queue<int> temp;
         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
         $$->type = type_c;
         $$->type->isZero = 0;
    }
  ;

logical_and_expression: 
    equality_expression
    {
         $$ = $1;  
    }
    | logical_and_expression AND_OP equality_expression
    {
         $$ = new op_binary_astnode("AND_OP", $1, $3);                  
         if(($1->type->isStruct==1 && ($1->type->deref_level+$1->type->array_level)==0)||($3->type->isStruct==1 && ($3->type->deref_level+$3->type->array_level)==0)){
              error(@$,"Invalid operand of &&, not scalar or pointer");
         }
         std::queue<int> temp;
         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
         $$->type = type_c;
         $$->type->isZero = 0;
    }
  ;

equality_expression: 
    relational_expression
    {
         $$ = $1;
    }
    | equality_expression EQ_OP relational_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 1);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("EQ_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("EQ_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary == , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;
    }
    | equality_expression NE_OP relational_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 1);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("NE_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("NE_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary != , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;
                 
    }
  ;

relational_expression: 
    additive_expression
    {
         $$ = $1;                    
    }
    | relational_expression '<' additive_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 0);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("LT_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("LT_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary < , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         } 
         $$->type->isZero = 0; 
     }
    | relational_expression '>' additive_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 0);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("GT_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("GT_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary > , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         } 
         $$->type->isZero = 0;     
    }
    | relational_expression LE_OP additive_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 0);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("LE_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("LE_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary <= , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;
    }
    | relational_expression GE_OP additive_expression
    {
         vector<exp_astnode*> received = checkComparison($1, $3, 0);
         $3 = received[1];
         $1 = received[0];
         if($3->type->valid == 1){
              $$ = new op_binary_astnode("GE_OP_INT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("int","",0,0,0,0,0,temp);
         }
         else if($3->type->valid == 2){
              $$ = new op_binary_astnode("GE_OP_FLOAT", $1, $3);
              std::queue<int> temp;
              $$->type = new type_class("float","",0,0,0,0,0,temp);
         }
         else{
               error(@$,"Invalid operand types for binary >= , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;            
    }
  ;

additive_expression: 
    multiplicative_expression
    {
         $$ = $1;
    }
    | additive_expression '+' multiplicative_expression
    {
         if($1->type->getString() == "int" && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("PLUS_INT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "float"){
              $$ = new op_binary_astnode("PLUS_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "int" && $3->type->getString() == "float"){
              $1 = new op_unary_astnode("TO_FLOAT", $1);
              $$ = new op_binary_astnode("PLUS_FLOAT", $1, $3);
              $$->type = new type_class($3->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "int"){
              $3 = new op_unary_astnode("TO_FLOAT", $3);
              $$ = new op_binary_astnode("PLUS_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if(($1->type->deref_level + $1->type->array_level) >0 && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("PLUS_INT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if(($3->type->deref_level + $3->type->array_level) >0 && $1->type->getString() == "int"){
              $$ = new op_binary_astnode("PLUS_INT", $1, $3);
              $$->type = new type_class($3->type);
              $$->type->lvalue = 0;
         }
         else{
               error(@$,"Invalid operand types for binary + , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;
     
         if($1->label > $3->label){
              $$->label = $1->label;
         }
         else if($1->label < $3->label){
              $$->label = $3->label;
         }
         else{
              $$->label = $1->label + 1;
         }
    
    }
    | additive_expression '-' multiplicative_expression
    {
         if($1->type->getString() == "int" && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("MINUS_INT", $1, $3);
               $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "float"){
              $$ = new op_binary_astnode("MINUS_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "int" && $3->type->getString() == "float"){
              $1 = new op_unary_astnode("TO_FLOAT", $1);
              $$ = new op_binary_astnode("MINUS_FLOAT", $1, $3);
              $$->type = new type_class($3->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "int"){
              $3 = new op_unary_astnode("TO_FLOAT", $3);
              $$ = new op_binary_astnode("MINUS_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if(($1->type->deref_level + $1->type->array_level) >0 && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("MINUS_INT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if(($1->type->deref_level + $1->type->array_level) >0 && ($3->type->deref_level + $3->type->array_level) >0){
              $$ = new op_binary_astnode("MINUS_INT", $1, $3);
              std::queue<int> x;
              $$->type = new type_class("int", "", 0,0,0,0,0,x);
         }
         else{
               error(@$,"Invalid operand types for binary - , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;

         if($1->label > $3->label){
              $$->label = $1->label;
         }
         else if($1->label < $3->label){
              $$->label = $3->label;
         }
         else{
              $$->label = $1->label + 1;
         }
    }
  ;

multiplicative_expression: 
    unary_expression
    {
         $$ = $1;
    }
    | multiplicative_expression '*' unary_expression
    {
         if($1->type->getString() == "int" && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("MULT_INT", $1, $3);
               $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "float"){
              $$ = new op_binary_astnode("MULT_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "int" && $3->type->getString() == "float"){
              $1 = new op_unary_astnode("TO_FLOAT", $1);
              $$ = new op_binary_astnode("MULT_FLOAT", $1, $3);
              $$->type = new type_class($3->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "int"){
              $3 = new op_unary_astnode("TO_FLOAT", $3);
              $$ = new op_binary_astnode("MULT_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else{
               error(@$,"Invalid operand types for binary * , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }

         if($1->label > $3->label){
              $$->label = $1->label;
         }
         else if($1->label < $3->label){
              $$->label = $3->label;
         }
         else{
              $$->label = $1->label + 1;
         }
         $$->type->isZero = 0;
    }
    | multiplicative_expression '/' unary_expression
    {
         if($1->type->getString() == "int" && $3->type->getString() == "int"){
              $$ = new op_binary_astnode("DIV_INT", $1, $3);
               $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "float"){
              $$ = new op_binary_astnode("DIV_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "int" && $3->type->getString() == "float"){
              $1 = new op_unary_astnode("TO_FLOAT", $1);
              $$ = new op_binary_astnode("DIV_FLOAT", $1, $3);
              $$->type = new type_class($3->type);
              $$->type->lvalue = 0;
         }
         else if($1->type->getString() == "float" && $3->type->getString() == "int"){
              $3 = new op_unary_astnode("TO_FLOAT", $3);
              $$ = new op_binary_astnode("DIV_FLOAT", $1, $3);
              $$->type = new type_class($1->type);
              $$->type->lvalue = 0;
         }
         else{
               error(@$,"Invalid operand types for binary / , \"" + $1->type->getString() + "\" and \"" + $3->type->getString() +"\"");
         }
         $$->type->isZero = 0;
    }
  ;

unary_expression: 
    postfix_expression
    {
         $$ = $1;
         multilevelstruct = false;
    }
    | unary_operator unary_expression
    {
          if($1 == "-"){
               $$ = new op_unary_astnode("UMINUS", $2);   
               if(!($2->type->getString()=="int" || $2->type->getString()=="float")){
                    error(@$,"Operand of unary - should be an int or float");
               }
               $$->type = new type_class($2->type);
              $$->type->lvalue = 0;
          }     
          else if($1 == "!"){
               $$ = new op_unary_astnode("NOT", $2);   
               std::queue<int> x;
               $$->type = new type_class("int", "", 0,0,0,0,0,x); 
          }
          else if($1 == "&"){
               $$ = new op_unary_astnode("ADDRESS", $2);   
               if($2->type->lvalue == 0){
                    error(@$,"Operand of & should  have lvalue");
               }
               $$->type = new type_class($2->type);
               $$->type->lvalue = 0;
               $$->type->array_level++;
               $$->type->bracketed = 1;
          }
          else if($1 == "*"){
               $$ = new op_unary_astnode("DEREF", $2);   
               if($2->type->deref_level + $2->type->array_level <= 0){
                    error(@$,"Invalid operand type \"" + $2->type->getString() + "\" of unary *");
               }
               if($2->type->getString() == "void*"){
                     error(@$,"Invalid operand type \"void*\" of unary *");
               }
               $$->type = new type_class($2->type);
               if($2->type->array_level > 0){
                    $$->type->array_level--;
                    if($$->type->bracketed == 1){
                         $$->type->bracketed = 0;
                         $$->type->lvalue = 1;
                    }
                    else{
                         $$->type->numElems.pop();
                    }
               }
               else{
                    $$->type->deref_level--;
               }
          }
          $$->type->isZero = 0;
    }
  ;

unary_operator: 
    '-'
    {
         $$ = "-";
    }
    | '!'
    {
         $$ = "!";
    }
    | '&'
    {
         $$ = "&";
    }
    | '*'
    {
         $$ = "*";
    }
 ;

postfix_expression:
    primary_expression
    {
         $$ = $1;  
         
    }
    | postfix_expression '[' expression ']'
    {
         if(($1->type->deref_level + $1->type->array_level) <= 0){
               error(@$,"Subscripted value is neither array nor pointer");
         }
         if($3->type->getString() != "int"){
              error(@$,"Array subscript is not an integer");
         }
          $$ = new arrayref_astnode($1, $3);
          $$->type = new type_class($1->type);
         if($$->type->array_level > 0){
              $$->type->array_level-=1;
              if($$->type->bracketed == 1){
                         $$->type->bracketed = 0;
                    }
                    else{
                         $$->type->numElems.pop();
                    }
         }
         else{
              $$->type->deref_level-=1;
         }
         if($$->type->array_level==0){
              $$->type->isConst=0;
         }
          $$->type->isZero = 0;
    }
    | IDENTIFIER '(' ')'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry == nullptr){
              if(predefined.find($1)!=predefined.end()){
                   $$ = new funcall_astnode(new identifier_astnode($1));
                   std::queue<int> temp;
                   std::string type = predefined.find($1)->second;
                   type_class* type_c = new type_class(type,"",0,0,0,0,0,temp);
                    $$->type = type_c;
              }
              else{
                   error(@$,"Function \"" + $1 + "\" not declared");
     
              }
          }
          else{
               std::vector<type_class*> params = receivedEntry->symbtab->getParams();

               if(params.size()!=0){
                    error(@$,"Function \"" + $1 + "\" called with too few arguments");
               }

               $$ = new funcall_astnode(new identifier_astnode($1));
               std::string typestr = receivedEntry->type;
               std::queue<int> temp;
               if(typestr == "int"){
                         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "float"){
                         type_class* type_c = new type_class("float","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "void"){
                         type_class* type_c = new type_class("void","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else{
                    type_class* type_c = new type_class(&typestr[7],"",1,0,0,0,0,temp);
                         $$->type = type_c;
               }
               $$->type->returned = 1;
          }
          $$->isFunction = 1;
          $$->type->isZero = 0;
    }
    | IDENTIFIER '(' expression_list ')'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry == nullptr){
              if(predefined.find($1)!=predefined.end()){
                   $3->insert(new identifier_astnode($1));
                    $$ = $3;
                   std::queue<int>temp;
                   type_class* type_c = new type_class(predefined[$1],"",0,0,0,0,0,temp);
                    $$->type = type_c;
              }
              else{
                   error(@$,"Function \"" + $1 + "\" not declared");
     
              }
 
         }
          else{
               std::vector<type_class*> params = receivedEntry->symbtab->getParams();
               
               if(params.size() > $3->arguments.size()){
                    error(@$,"Function \"" + $1 + "\" called with too few arguments");
               }
               else if(params.size() < $3->arguments.size()){
                    error(@$,"Function \"" + $1 + "\" called with too many arguments");
               }

               for (int i=0;i<$3->arguments.size();i++){
                    
                    exp_astnode* newnode = new intconst_astnode("1");
                    newnode->type = new type_class(params[i]);
                    newnode->type->lvalue = 1;

                    newnode = checkAssign(newnode,$3->arguments[i],0);

                    
                    if(newnode->type->valid!=1){
                         type_class * type_c1 = new type_class(params[i]);
                         if(type_c1->array_level > 0){
                              
                              type_c1->numElems.pop();
                              if(type_c1->array_level==1){
                                   type_c1->array_level--;
                                   type_c1->deref_level++;
                              }
                              else{
                                   type_c1->bracketed=1;
                              }
                         }

                        type_class * type_c2 = new type_class($3->arguments[i]->type);
                         
                        if(type_c2->array_level > 0 && type_c2->bracketed == 0){
                             std::cout<<type_c2->getString()<<endl;
                             
                             type_c2->numElems.pop();
                             
                              if(type_c2->array_level==1){
                                   type_c2->array_level--;
                                   type_c2->deref_level++;
                              }
                              else{
                                   type_c2->bracketed=1;
                              }
                        }
                         
                        error(@$,"Expected \"" + type_c1->getString() + "\" but argument is of type \"" + type_c2->getString() + "\"");
                    }
                       
                    $3->arguments[i] = newnode;  
               }     
               
               $3->insert(new identifier_astnode($1));
                $$ = $3;
                $$->isFunction = 1;
                std::string typestr = receivedEntry->type;
               std::queue<int> temp;
               if(typestr == "int"){
                         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "float"){
                         type_class* type_c = new type_class("float","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "void"){
                         type_class* type_c = new type_class("void","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else{
                    type_class* type_c = new type_class(&typestr[7],"",1,0,0,0,0,temp);
                         $$->type = type_c;
               }
               $$->type->returned = 1;
          }
          $$->type->isZero = 0;
          
    }
    | postfix_expression {
         
         if(!($1->type->isStruct && ($1->type->deref_level + $1->type->array_level == 0)))
         {
              error(@$,"Left operand of \".\"  is not a  structure");
         }
         const EntryGlobal* receivedStructEntry = gst.getStructEntry("struct "+$1->type->type);
         
         const EntryLocal* receivedEntry;
         if(multilevelstruct){
              receivedEntry = temprunningST->getEntry($1->type->name);
         }
         else{
              receivedEntry = runningST->getEntry($1->type->name);
         }
         
         if(receivedEntry == nullptr && !$1->type->returned){
              error(@$,"Variable \"" + $1->type->name + "\" not declared");
         }
         temprunningST = receivedStructEntry->symbtab;
     

    }'.' IDENTIFIER
    {
         multilevelstruct = true;
         const EntryGlobal* receivedStructEntry = gst.getStructEntry("struct "+$1->type->type);
          const EntryLocal* receivedLocalEntry = receivedStructEntry->symbtab->getEntry($4);
          if(receivedLocalEntry == nullptr){               
               error(@$,"Struct \"struct " + $1->type->type + "\" has no member named \"" + $4 + "\"");
          }

         $$ = new member_astnode($1, new identifier_astnode($4));
         $$->type = receivedLocalEntry->type_c;
         $$->type->isZero = 0;
    }
    | postfix_expression PTR_OP IDENTIFIER
    {    if(!($1->type->isStruct && ($1->type->deref_level + $1->type->array_level == 1)))
         {
              error(@$,"Left operand of \"->\" is not a pointer to structure");
         }
         const EntryLocal* receivedEntry = runningST->getEntry($1->type->name);
         if(receivedEntry == nullptr){
              error(@$,"Variable \"" + $1->type->name + "\" not declared");
         }
          const EntryGlobal* receivedStructEntry = gst.getStructEntry("struct "+$1->type->type);
          const EntryLocal* receivedLocalEntry= receivedStructEntry->symbtab->getEntry($3);
          
          if(receivedLocalEntry == nullptr){
               
               error(@$,"Struct \"struct " + $1->type->type + "\" has no member named \"" + $3 + "\"");
          }

         $$ = new arrow_astnode($1, new identifier_astnode($3));
         $$->type = receivedLocalEntry->type_c;
         $$->type->isZero = 0;
    }
    | postfix_expression INC_OP
    {
         if(!$1->type->lvalue){
               error(@$,"Left operand of assignment should have an lvalue");
         }
         if(!(!($1->type->isStruct) && $1->type->array_level == 0)){
              error(@$,"Operand of \"++\" should be a int, float or pointer");
         }
         $$ = new op_unary_astnode("PP", $1);
         $$->type = new type_class($1->type);
         $$->type->lvalue = 0;
         $$->type->isZero = 0;
    }
  ;

program:
  main_definition
  {
    $$ = $1;
  }
  | translation_unit main_definition
  {
     $$ = $1;
  }


translation_unit: 
    struct_specifier
    {
         $$ = $1;
    }
    | function_definition
    {
         $$ = $1;
    }
    | translation_unit struct_specifier
    {
        $$ = $1;
    }
    | translation_unit function_definition
    {
        $$ = $1;
    }
  ;

struct_specifier: 
    STRUCT IDENTIFIER { runningST = new LocalSymbTab(); running_offset = 0; struct_running = 1; 
          std::string struct_name = "struct " + $2;
          int total_size = 0;
          newEntryG = new EntryGlobal(struct_name,"struct","global",total_size,-1,"-",runningST);
          gst.Entries.insert({struct_name,*newEntryG});
    
    } '{' declaration_list '}' ';'
    {
        std::string struct_name = "struct " + $2;
        int total_size = 0;
        for (const auto &entry : runningST->Entries)
        {
             total_size += entry.size;
        }        
        gst.setStructSize(struct_name,total_size);
        gst.setSymbolTable(struct_name, runningST);
        struct_running = 0;
    }
  ;

main_definition: 
     INT {
         type_specifier_class* temporary = new type_specifier_class("int", 0);     
         running_fun_type = temporary; runningST = new LocalSymbTab(); running_offset = 8;
     } 
     MAIN '(' ')' {
          funcname = $3; 
          newEntryG = new EntryGlobal(funcname,"fun","global",0,0,"int",nullptr);
          gst.Entries.insert({funcname,*newEntryG});
          queue<int> empty;
          param_sizes = empty;
          running_offset = 0;
     } 
     compound_statement{

     }

function_definition: 
    type_specifier { 
         running_fun_type = $1; runningST = new LocalSymbTab(); running_offset = 8;
    } 
    fun_declarator { 
          funcname = $3->name; 
          std::string type_str="";
          int total_size=4;
          if(running_fun_type->isStruct){
               type_str = type_str + "struct ";
          }
          type_str = type_str + running_fun_type->type;

          newEntryG = new EntryGlobal(funcname,"fun","global",0,0,type_str,nullptr);
          gst.Entries.insert({funcname,*newEntryG});
          runningST->setParamSizes(param_sizes,running_offset);
          queue<int> empty;
          param_sizes = empty;
          running_offset = 0;
     } 
     compound_statement
    {
        
    }
  ;

type_specifier: 
    VOID
    {
         $$ = new type_specifier_class("void",0);
    }
    | INT
    {
         $$ = new type_specifier_class("int",0);
    }
    | FLOAT
    {
         $$ = new type_specifier_class("float",0);
    }
    | STRUCT IDENTIFIER
    {     const EntryGlobal* received_entry = gst.getStructEntry("struct " + $2);
          if(received_entry==nullptr){
               error(@$,"struct " + $2 + " is not defined");
          }
          $$ = new type_specifier_class($2,1);
    }
  ;

fun_declarator: 
    IDENTIFIER '(' parameter_list ')'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry!=nullptr){
              error(@$,"The function \"" + $1 + "\" has a previous definition");
         }

         $$ = new fun_declarator_class($1,$3);
    }
    | IDENTIFIER '(' ')'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry!=nullptr){
              error(@$,"The function \"" + $1 + "\" has a previous definition");
         }

         $$ = new fun_declarator_class($1);
    }
  ;

parameter_list: 
    parameter_declaration
    {
          $$ =  new parameter_list_class($1);          
    }
    | parameter_list ',' parameter_declaration
    {
          $1->insert($3);
          $$ = $1;
    }
  ;

parameter_declaration: 
    type_specifier declarator
    {     
          int size_type,num_elem=1,total_size;
          
          type_class* type_c = new type_class($1->type,$2->name,$1->isStruct,$2->deref_level,$2->array_level,1,0,$2->numElems);

          if(($1->isStruct==1) && (gst.getStructEntry("struct " + $1->type)==nullptr)){
               error(@$,"struct " + $1->type + " is not defined");
          }
          else if($1->type=="void" && $2->deref_level==0){
               error(@$,"Cannot declare variable of type \"void\"");
          }
          else if(runningST->getEntry($2->name)!=nullptr){
               error(@$,"\"" + $2->name + "\" has a previous declaration");
          }

          if($1->isStruct){
               size_type = gst.getStructEntry("struct "+ $1->type)->size;
               if($2->deref_level>0){
                    size_type = 4;
               }
          }
          else{
               size_type = 4;
          }

          /*
          if($2->array_level>0){

               queue<int> replacement = $2->numElems;
               while(!replacement.empty()){
                    int val = replacement.front();
                    replacement.pop();
                    num_elem = num_elem*val;
               }
          }
          */
          
          total_size = size_type*num_elem;
          param_sizes.push(total_size);
          newEntryL = new EntryLocal($2->name,"var","param",total_size,0,type_c->getString(),type_c);
          runningST->Entries.push_back(*newEntryL);

          $$ = new parameter_declaration_class($1,$2);     
          running_offset += total_size;                  
    }
  ;

declarator_arr: 
    IDENTIFIER
    {
        $$ = new declarator_class($1);
    }
    | declarator_arr '[' INT_CONSTANT ']'
    {
        $1->inc_arr($3);
        $$ = $1;
    }
  ;

declarator: 
    declarator_arr
    {
          $$ = $1;               
    }
    | '*' declarator
    {
          $2->deref_level = $2->deref_level+1;
          $$ = $2;
    }
  ;

compound_statement: 
    '{' {gst.setSymbolTable(funcname,runningST);} '}'
    { 
         $$ = new seq_astnode();
         if(funcname=="main"){
              $$->isMain=1;
         }
         if(hasReturn==1){
             $$->hasReturn = 1;
             hasReturn = 0;
         }
         ast.insert({funcname,$$});   
    }
    | '{' {gst.setSymbolTable(funcname,runningST);} statement_list '}'
    {

         $$ = $3;
         if(funcname=="main"){
              $$->isMain=1;
         }
         if(hasReturn==1){
             $$->hasReturn = 1;
             hasReturn = 0;
         }
         ast.insert({funcname,$$});   
    }
    | '{' declaration_list '}'
    {
         $$ = new seq_astnode();
         if(funcname=="main"){
              $$->isMain=1;
         }
         if(hasReturn==1){
             $$->hasReturn = 1;
             hasReturn = 0;
         }
         ast.insert({funcname,$$});
         gst.setSymbolTable(funcname,runningST);  
    }
    | '{' declaration_list {gst.setSymbolTable(funcname,runningST); } statement_list '}'
    {
        $$ = $4;
        if(funcname=="main"){
             $$->isMain=1;
        }
        if(hasReturn==1){
             $$->hasReturn = 1;
             hasReturn = 0;
        }
        ast.insert({funcname,$$}); 
    }
  ;  

statement_list: 
    statement
    {
          $$ = new seq_astnode($1);              
    }
    | statement_list statement
    {
          $1->insert($2);
          $$ = $1;     
    }
  ;

statement: 
    ';'
    {
          $$ = new empty_astnode();
    }
    | '{' statement_list '}'
    {
          $$ = $2;
     }
    | selection_statement
    {
          $$ = $1;          
    }

    | iteration_statement
    {
          $$ = $1 ;              
    }     

    | assignment_statement
    {
          $$ = $1;             
    }

    | procedure_call
    {
         $$ = $1;
    }
    | printf_call
    {
         $$ = $1;
    }
    | RETURN expression ';'
    {
         exp_astnode* newnode = new intconst_astnode("1");
         newnode->type = new type_class(running_fun_type->type,"",running_fun_type->isStruct,0,0,1,0);
         exp_astnode* node = checkAssign(newnode,$2,0);
         if(node->type->valid==0){
              error(@$,"Incompatible type \"" + node->type->getString() + "\" returned, expected \"" + newnode->type->getString() + "\"");
         }
         $$ = new return_astnode(node);
         hasReturn = 1;
    }
  ;

assignment_expression: 
    unary_expression '=' expression
    {
         exp_astnode* node = checkAssign($1,$3,0);
         if(node->type->valid==-1){
              error(@$,"Left operand of assignment should have an lvalue");
         }
         else if(node->type->valid==0){
              error(@$,"Incompatible assignment when assigning to type \"" + $1->type->getString() + "\" from type \"" + $3->type->getString() + "\"");
         }
         $$ = new assignE_astnode($1, node);
    }
  ;


assignment_statement: 
    assignment_expression ';'
    {
          $$ = new assignS_astnode($1->left,$1->right);        
    }
  ;

procedure_call: 
    IDENTIFIER '(' ')' ';'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry == nullptr){
              if(predefined.find($1)!=predefined.end()){
                   $$ = new proccall_astnode(new funcall_astnode(new identifier_astnode($1)));
                   /*std::queue<int> temp;
                   type_class* type_c = new type_class(predefined[$1],"",0,0,0,0,0,temp);
                    $$->type = type_c;*/
              }
              else{
                   error(@$,"Function \"" + $1 + "\" not declared");
     
              }
         }
          else{             
               std::vector<type_class*> params = receivedEntry->symbtab->getParams();

               if(params.size()!=0){
                    error(@$,"Procedure \"" + $1 + "\" called with too few arguments");
               }

               $$ = new proccall_astnode(new funcall_astnode(new identifier_astnode($1)));
               /*
               string typestr = receivedEntry->type;
               std::queue<int> temp;
               if(typestr == "int"){
                         type_class* type_c = new type_class("int","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "float"){
                         type_class* type_c = new type_class("float","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else if(typestr == "void"){
                         type_class* type_c = new type_class("void","",0,0,0,0,0,temp);
                         $$->type = type_c;
               }
               else{
                    type_class* type_c = new type_class(typestr,"",1,0,0,0,0,temp);
                         $$->type = type_c;
               }
               */
          }

    }
    | IDENTIFIER '(' expression_list ')' ';'
    {
         const EntryGlobal* receivedEntry = gst.getFuncEntry($1);

         if(receivedEntry == nullptr){
              if(predefined.find($1)!=predefined.end()){
                   $3->insert(new identifier_astnode($1));
                    $$ = new proccall_astnode($3);      
                   /*type_class* type_c = new type_class(predefined.find($1)->second,"",0,0,0,0,0,temp);
                    $$->type = type_c;*/
              }
              else{
                   error(@$,"Function \"" + $1 + "\" not declared");
     
              }
         }
          else{               
               std::vector<type_class*> params = receivedEntry->symbtab->getParams();
               
               if(params.size() > $3->arguments.size()){
                    error(@$,"Procedure \"" + $1 + "\" called with too few arguments");
               }
               else if(params.size() < $3->arguments.size()){
                    error(@$,"Procedure \"" + $1 + "\" called with too many arguments");
               }
               for (int i=0;i<$3->arguments.size();i++){
                    exp_astnode* newnode = new intconst_astnode("1");
                    newnode->type = new type_class(params[i]);
                    newnode->type->lvalue = 1;

                    newnode = checkAssign(newnode,$3->arguments[i],0);
                    
                    if(newnode->type->valid!=1){
                         type_class * type_c1 = new type_class(params[i]);
                         if(type_c1->array_level > 0){
                              
                              type_c1->numElems.pop();
                              if(type_c1->array_level==1){
                                   type_c1->array_level--;
                                   type_c1->deref_level++;
                              }
                              else{
                                   type_c1->bracketed=1;
                              }
                         }

                        type_class * type_c2 = new type_class($3->arguments[i]->type);
                         
                        if(type_c2->array_level > 0 && type_c2->bracketed == 0){
                             std::cout<<type_c2->getString()<<endl;
                             
                             type_c2->numElems.pop();
                             
                              if(type_c2->array_level==1){
                                   type_c2->array_level--;
                                   type_c2->deref_level++;
                              }
                              else{
                                   type_c2->bracketed=1;
                              }
                        }
                         
                        error(@$,"Expected \"" + type_c1->getString() + "\" but argument is of type \"" + type_c2->getString() + "\"");
                    }
                       
                    $3->arguments[i] = newnode;   
               }     

               $3->insert(new identifier_astnode($1));
               $$ = new proccall_astnode($3);      
          } 
    }
  ;

printf_call: 
  PRINTF '(' STRING_LITERAL ')' ';'
  {
     stringconst_astnode* temp_str; 
     if(LocalStrings.find(funcname)!=LocalStrings.end()){
           int lc;
           if(LocalStrings[funcname].find($3) == LocalStrings[funcname].end()){
               lc = LC_Count;
               LocalStrings[funcname].insert({$3,LC_Count});
               LC_Count++;
           }
           else{
               lc = LocalStrings[funcname][$3];
           }
           temp_str = new stringconst_astnode(lc);
           std::queue<int> temp;
           type_class* type_c = new type_class("string","",0,0,0,0,0,temp);
           temp_str->type = type_c;
     }
     else{
          map<string,int> new_map;
          temp_str = new stringconst_astnode(LC_Count);
          std::queue<int> temp;
          type_class* type_c = new type_class("string","",0,0,0,0,0,temp);
          temp_str->type = type_c;
          new_map.insert({$3,LC_Count});
          LC_Count++;
          LocalStrings.insert({funcname,new_map});
     }
     funcall_astnode* temporary = new funcall_astnode(temp_str);                
     temporary->insert(new identifier_astnode($1));
     $$ = new proccall_astnode(temporary);          
  }
  | PRINTF '(' STRING_LITERAL ',' expression_list ')' ';'
  {               
     stringconst_astnode* temp_str; 
     if(LocalStrings.find(funcname) != LocalStrings.end()){
           int lc;
           if(LocalStrings[funcname].find($3) == LocalStrings[funcname].end()){
               lc = LC_Count;
               LocalStrings[funcname].insert({$3,LC_Count});
               LC_Count++;
           }
           else{
               lc = LocalStrings[funcname][$3];
           }
           temp_str = new stringconst_astnode(lc);
           std::queue<int> temp;
           type_class* type_c = new type_class("string","",0,0,0,0,0,temp);
           temp_str->type = type_c;
     }
     else{
          map<string,int> new_map;
          temp_str = new stringconst_astnode(LC_Count);
          std::queue<int> temp;
          type_class* type_c = new type_class("string","",0,0,0,0,0,temp);
          temp_str->type = type_c;
          new_map.insert({$3,LC_Count});
          LC_Count++;
          LocalStrings.insert({funcname,new_map});
     }
     $5->insert_front(temp_str);
     $5->insert(new identifier_astnode($1));
     $$ = new proccall_astnode($5); 
  }

expression_list: 
    expression
    {
         $$ = new funcall_astnode($1);                    
    }
    | expression_list ',' expression
    {
         $1->insert($3); 
         $$ = $1;                   
    }
 ;



selection_statement: 
    IF '(' expression ')' statement ELSE statement
    {
         /*REMAINS*/
         $$ = new if_astnode($3, $5, $7);
    }
 ;

iteration_statement: 
    WHILE '(' expression ')' statement
    {
         /*REMAINS*/
         $$ = new while_astnode($3, $5);
    }
    | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement
    {
         /*REMAINS*/
         $$ = new for_astnode($3, $5, $7, $9);
    }
 ;

declaration_list: 
    declaration
    {
         $$ = new declaration_list_class($1);
    }
    | declaration_list declaration
    {
         $1->insert($2);
         $$ = $1;
    }
 ;

declaration: 
    type_specifier { running_local_type = $1;} declarator_list ';'
    {
         $$ = new declaration_class($1, $3);
    }

declarator_list: 
    declarator
    {
          int size_type,num_elem=1,total_size,isConst;
          
          if($1->array_level>0){
               isConst=1;
          }
          else isConst=0;

          type_class* type_c = new type_class(running_local_type->type,$1->name,running_local_type->isStruct,$1->deref_level,$1->array_level,1,isConst,$1->numElems);

          if((running_local_type->isStruct==1) && (gst.getStructEntry("struct " + running_local_type->type)==nullptr)){
               error(@$,"struct " + running_local_type->type + " is not defined");
          }
          else if(running_local_type->type=="void" && $1->deref_level==0){
               error(@$,"Cannot declare variable of type \"void\"");
          }
          else if(runningST->getEntry($1->name)!=nullptr){
               error(@$,"\"" + $1->name + "\" has a previous declaration");
          }

          if(running_local_type->isStruct){
               size_type = gst.getStructEntry("struct "+ running_local_type->type)->size;
               if($1->deref_level>0){
                    size_type = 4;
               }
          }
          else{
               size_type = 4;
          }

          if($1->array_level>0){                     
               queue<int> replacement = $1->numElems;
               while(!replacement.empty()){
                    int val = replacement.front();
                    replacement.pop();
                    num_elem = num_elem*val;
               }
          }

          total_size = size_type*num_elem;

          if(struct_running!=1)
               running_offset -= total_size;                  

          newEntryL = new EntryLocal($1->name,"var","local",total_size,running_offset,type_c->getString(),type_c);
          runningST->Entries.push_back(*newEntryL);

          if(struct_running==1)
               running_offset += total_size;                  

          $$ = new declarator_list_class($1);

    }
    | declarator_list ',' declarator
    {
          int size_type,num_elem=1,total_size,isConst;
          
          if($3->array_level>0){
               isConst=1;
          }
          else isConst=0;

          type_class* type_c = new type_class(running_local_type->type,$3->name,running_local_type->isStruct,$3->deref_level,$3->array_level,1,isConst,$3->numElems);

          if((running_local_type->isStruct==1) && (gst.getStructEntry("struct " + running_local_type->type)==nullptr)){
               error(@$,"struct " + running_local_type->type + " is not defined");
          } 
          else if(running_local_type->type=="void"  && $3->deref_level==0){
               error(@$,"Cannot declare variable of type \"void\"");
          }
          else if(runningST->getEntry($3->name)!=nullptr){
               error(@$,"\"" + $3->name + "\" has a previous declaration");
          }


          if(running_local_type->isStruct){
               size_type = gst.getStructEntry("struct "+ running_local_type->type)->size;
               if($3->deref_level>0){
                    size_type = 4;
               }
          }
          else{
               size_type = 4;
          }

          if($3->array_level>0){               
               queue<int> replacement = $3->numElems;
               while(!replacement.empty()){
                    int val = replacement.front();
                    replacement.pop();
                    num_elem = num_elem*val;
               }
          }

          total_size = size_type*num_elem;

          if(struct_running!=1)
               running_offset -= total_size;                  

          newEntryL = new EntryLocal($3->name,"var","local",total_size,running_offset,type_c->getString(),type_c);          
          runningST->Entries.push_back(*newEntryL);
          $1->insert($3);

          if(struct_running==1)
               running_offset += total_size;                  

          $$ = $1;
    }
 ;

%%

void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cout << "Error at line " << l.begin.line << ": "<< err_message<<"\n";
   exit(1);
}


