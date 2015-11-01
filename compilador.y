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
%token AND OR SUM SUB TIMES DIV MOD TRUE FALSE

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
  { line_variables = 0; }
  variable_list COLON type  {
    set_last_symbols_type(line_variables, symbol_type_id);
    generate_code(NULL, "AMEM %u", line_variables);
  }
  SEMICOLON
;

type :
  IDENT {
    symbol_type_id = parse_type(token);
  }
;

variable_list :
  variable_list COMMA
  IDENT {
    create_symbol(token, variable_symbol), ++line_variables;
  }
  | /* OR */
  IDENT {
    create_symbol(token, variable_symbol), ++line_variables;
  }
;

ident_list :
  ident_list COMMA IDENT
  | /* OR */ 
  IDENT
;

variable :
  IDENT {
    strncpy(variable_name, token, sizeof variable_name);
    variable_reference = get_symbol_reference(token);
  }
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
  variable SET expression {
    process_stack_type(&expr_stack, find_symbol(variable_name)->sym_type, NULL);
    generate_code(NULL, "ARMZ %s", variable_reference);
  }
;

function_call :
  IDENT {
    function_reference = get_symbol_reference(token);
  }
  PARENTHESES_OPEN expression_list PARENTHESES_CLOSE
;

if_statement:
  IF expression {
    if_not_label = get_next_label();
    process_stack_type(&expr_stack, sym_type_boolean, NULL);
    generate_code(NULL, "DSVF %s", get_label_string(if_not_label));
    uipush(&if_stack, if_not_label);
  }
  THEN composed_instructions
;

conditional_instruction :
  if_statement %prec LOWER_THAN_ELSE {
    generate_label(uipop(&if_stack));
  }
  | /* OR */
  if_statement ELSE {
    if_label = get_next_label();
    generate_code(NULL, "DSVS %s", get_label_string(if_label));
    generate_label(uipop(&if_stack));
    uipush(&if_stack, if_label);
  }
  composed_instructions {
    generate_label(uipop(&if_stack));
  }
;

loop_instruction :
  WHILE {
    while_inner_label = get_next_label();
    while_outter_label = get_next_label(); 
    generate_label(while_inner_label);
  }
  expression {
    process_stack_type(&expr_stack, sym_type_boolean, NULL);
    generate_code(NULL, "DSVF %s", get_label_string(while_outter_label));
    uipush(&while_stack, while_inner_label);
    uipush(&while_stack, while_outter_label);
  }
  DO composed_instructions {
    while_outter_label = uipop(&while_stack);
    while_inner_label = uipop(&while_stack);
    generate_code(NULL, "DSVS %s", get_label_string(while_inner_label));
    generate_label(while_outter_label);
  }
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

expression :
  simple_expression
  | /* OR */
  expression relation_operator {
    relation = symbol;
  }
  simple_expression
;

simple_expression :
  term {
    transfer_stack_type(&term_stack, &expr_stack);
  }
  | /* OR */
  SUM term {
    process_stack_type(&term_stack, sym_type_integer, &expr_stack);
  }
  | /* OR */
  SUB term {
    process_stack_type(&term_stack, sym_type_integer, &expr_stack);
    generate_code(NULL, "INVR");
  }
  | /* OR */
  simple_expression SUM term {
    process_stack_type(&expr_stack, sym_type_integer, NULL);
    process_stack_type(&term_stack, sym_type_integer, &expr_stack);
    generate_code(NULL, "SOMA");
  }
  | /* OR */
  simple_expression SUB term {
    process_stack_type(&expr_stack, sym_type_integer, NULL);
    process_stack_type(&term_stack, sym_type_integer, &expr_stack);
    generate_code(NULL, "SUB");
  }
  | /* OR */
  simple_expression OR term {
    process_stack_type(&expr_stack, sym_type_boolean, NULL);
    process_stack_type(&term_stack, sym_type_boolean, &expr_stack);
    generate_code(NULL, "DISJ");
  }
;

term :
  factor {
    transfer_stack_type(&factor_stack, &term_stack);
  }
  | /* OR */
  term TIMES factor {
    process_stack_type(&term_stack, sym_type_integer, NULL);
    process_stack_type(&factor_stack, sym_type_integer, &term_stack);
    generate_code(NULL, "MULT");
  }
  | /* OR */
  term DIV factor {
    process_stack_type(&term_stack, sym_type_integer, NULL);
    process_stack_type(&factor_stack, sym_type_integer, &term_stack);
    generate_code(NULL, "DIVI");
  }
  | /* OR */
  term AND factor {
    process_stack_type(&term_stack, sym_type_boolean, NULL);
    process_stack_type(&factor_stack, sym_type_boolean, &term_stack);
    generate_code(NULL, "CONJ");
  }
;

factor :
  NOT factor {
    process_stack_type(&factor_stack, sym_type_boolean, &factor_stack);
    generate_code(NULL, "NEGA");
  }
  | /* OR */
  variable {
    ipush(&factor_stack, (int) find_symbol(variable_name)->sym_type);
    generate_code(NULL, "CRVL %s", variable_reference);
  }
  | /* OR */
  CONSTANT {
    ipush(&factor_stack, (int) sym_type_integer);
    generate_code(NULL, "CRCT %s", token);
  }
  | /* OR */
  TRUE {
    ipush(&factor_stack, (int) sym_type_boolean);
    generate_code(NULL, "CRCT 1");
  }
  | /* OR */
  FALSE {
    ipush(&factor_stack, (int) sym_type_boolean);
    generate_code(NULL, "CRCT 0");
  }
  | /* OR */
  function_call
  | /* OR */
  PARENTHESES_OPEN expression PARENTHESES_CLOSE {
    transfer_stack_type(&expr_stack, &factor_stack);
  }
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

