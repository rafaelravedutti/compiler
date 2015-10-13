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
  { generate_code(NULL, "AMEM %u", line_variables); } SEMICOLON
;

type :
  IDENT
;

variable_list :
  variable_list COMMA
  IDENT { create_symbol(token, sym_type_var), ++line_variables; }
  | /* OR */
  IDENT { create_symbol(token, sym_type_var), ++line_variables; }
;

ident_list :
  ident_list COMMA IDENT
  | /* OR */ 
  IDENT
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
  IDENT { ident_ref = get_symbol_ref(token); } ident_instruction
  | /* OR */
  WHILE expression DO composed_instructions
  | /* OR */
  IF expression THEN composed_instructions
  | /* OR */
  FUNCTION IDENT PARENTHESES_OPEN declare_vars_block PARENTHESES_CLOSE
  | /* OR */
  PROCEDURE IDENT PARENTHESES_OPEN declare_vars_block PARENTHESES_CLOSE 
;

ident_instruction :
  SET expression { generate_code(NULL, "ARMZ %s", ident_ref); }
  | /* OR */
  PARENTHESES_OPEN ident_list PARENTHESES_CLOSE
;

expression :
  expression2
  | /* OR */
  NOT expression2
  | /* OR */
  expression AND expression2
  | /* OR */
  expression OR expression2
;

expression2 :
  expression3
  | /* OR */
  expression2 EQUAL expression3
  | /* OR */
  expression2 DIFF expression3
  | /* OR */
  expression2 LESS_THAN expression3
  | /* OR */
  expression2 HIGHER_THAN expression3
  | /* OR */
  expression2 LESS_OR_EQUAL_THAN expression3
  | /* OR */
  expression2 HIGHER_OR_EQUAL_THAN expression3

expression3 :
  expression4
  | /* OR */
  SUB expression4
  | /* OR */
  expression3 SUM expression4
  | /* OR */
  expression3 SUB expression4
;

expression4 :
  expression5
  | /* OR */
  expression4 TIMES expression5
  | /* OR */
  expression4 DIV expression5
  | /* OR */
  expression4 MOD expression5
;

expression5 :
  IDENT { generate_code(NULL, "CRVL %s", get_symbol_ref(token)); }
  | /* OR */
  CONSTANT { generate_code(NULL, "CRCT %s", token); }
  | /* OR */
  PARENTHESES_OPEN expression PARENTHESES_CLOSE
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
    return(-1);
  }

  yyin = fp;
  yyparse();

  free_symbols();
  return 0;
}

