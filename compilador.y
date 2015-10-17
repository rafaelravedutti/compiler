%{

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"

%}

%token PROGRAM PARENTHESES_OPEN PARENTHESES_CLOSE 
%token COMMA SEMICOLON COLON DOT VAR IDENT SET
%token T_BEGIN T_END WHILE DO FOR TO DOWNTO IF THEN ELSE
%token PROCEDURE FUNCTION REPEAT UNTIL GOTO LABEL NOT
%token CASE IN CONSTANT EQUAL DIFF LESS_THAN HIGHER_THAN
%token LESS_OR_EQUAL_THAN HIGHER_OR_EQUAL_THAN
%token AND OR SUM SUB TIMES DIV MOD

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program :  
  { generate_code(NULL, "INPP"); }
  PROGRAM IDENT PARENTHESES_OPEN ident_list PARENTHESES_CLOSE SEMICOLON block DOT
  { generate_code(NULL, "PARA"); }
;

block :
  variable_declaration_part
  composed_instructions
  { generate_code(NULL, "DMEM %u", free_level_symbols()); } 
;

/* Declaração de variáveis */
variable_declaration_part :
  { block_variables = 0; }
  VAR declare_vars_block
  | /* OR */
  /* Nothing */
;

declare_vars_block :
  declare_vars_block declare_vars_line
  | /* OR */
  declare_vars_line
;

declare_vars_line :
  { line_variables = 0; } variable_list COLON type 
  { set_last_symbols_type(line_variables, symbol_type_id);
    generate_code(NULL, "AMEM %u", line_variables); } SEMICOLON
;

type :
  IDENT { symbol_type_id = parse_type(token); }
;

variable_list :
  variable_list COMMA
  IDENT { create_symbol(token, variable_symbol), ++line_variables; }
  | /* OR */
  IDENT { create_symbol(token, variable_symbol), ++line_variables; }
;

ident_list :
  ident_list COMMA IDENT
  | /* OR */ 
  IDENT
;

variable :
  IDENT { variable_reference = get_symbol_reference(token); }
;

/* Comandos */
composed_instructions :
  T_BEGIN instructions T_END 
;

instructions :
  instruction SEMICOLON instructions
  | /* OR */
  /* Nothing */
; 

instruction :
  set_instruction
  | /* OR */
  function_call
  | /* OR */
  conditional_instruction
  | /* OR */
  loop_instruction
;

set_instruction :
  variable SET expression { generate_code(NULL, "ARMZ %s", variable_reference); }
;

function_call:
  IDENT { function_reference = get_symbol_reference(token); }
  PARENTHESES_OPEN expression_list PARENTHESES_CLOSE
;

conditional_instruction :
  IF bool_expression THEN composed_instructions %prec LOWER_THAN_ELSE
  | /* OR */
  IF bool_expression THEN composed_instructions ELSE composed_instructions
;

loop_instruction :
  WHILE bool_expression DO composed_instructions
;

relation_operator :
  EQUAL
  | /* OR */
  DIFF
  | /* OR */
  LESS_THAN
  | /* OR */
  LESS_OR_EQUAL_THAN
  | /* OR */
  HIGHER_THAN
  | /* OR */
  HIGHER_OR_EQUAL_THAN
;

bool_expression :
  expression
  | /* OR */
  bool_expression relation_operator { relation = symbol; } expression
;

expression :
  term
  | /* OR */
  SUM term
  | /* OR */
  SUB term { generate_code(NULL, "INVR"); }
  | /* OR */
  expression SUM term { generate_code(NULL, "SOMA"); }
  | /* OR */
  expression SUB term { generate_code(NULL, "SUB"); }
  | /* OR */
  expression OR term { generate_code(NULL, "DISJ"); }
;

term :
  factor
  | /* OR */
  term TIMES factor { generate_code(NULL, "MULT"); }
  | /* OR */
  term DIV factor { generate_code(NULL, "DIVI"); }
  | /* OR */
  term AND factor { generate_code(NULL, "CONJ"); }
;

factor :
  NOT factor { generate_code(NULL, "NEGA"); }
  | /* OR */
  variable { generate_code(NULL, "CRVL %s", variable_reference); }
  | /* OR */
  CONSTANT { generate_code(NULL, "CRCT %s", token); }
  | /* OR */
  function_call
  | /* OR */
  PARENTHESES_OPEN expression PARENTHESES_CLOSE
;

expression_list:
  expression
  | /* OR */
  expression COMMA expression
;


%%

int main (int argc, const char *argv[]) {
  FILE *fp;
  extern FILE *yyin;

  if(argc != 2) {
    fprintf(stdout, "Uso: %s <arquivo>\n", argv[0]);
    return -1;
  }

  if((fp = fopen(argv[1], "r")) == NULL) {
    fprintf(stdout, "Ocorreu um erro ao abrir o arquivo!\n");
    return -1;
  }

  yyin = fp;
  yyparse();

  free_symbols();
  return 0;
}

