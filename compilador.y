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
%token WRITE READ DIV_INT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program : {
    generate_code(NULL, "INPP");
  }
  PROGRAM IDENT PARENTHESES_OPEN ident_list PARENTHESES_CLOSE SEMICOLON block DOT {
    generate_code(NULL, "PARA");
  }
;

block :
  label_declaration_part
  variable_declaration_part
  subroutine_declaration_part
  composed_instructions {
    free_level_symbols(lexical_level);
  } 
;

/* Declaração de rótulos */
label_declaration_part :
  LABEL label_list
  | /* OR */
  /* Nothing */
;

label_list :
  CONSTANT {
    create_symbol(token, label_symbol, get_next_label());
  }
  COMMA label_list
  | /* OR */
  CONSTANT {
    create_symbol(token, label_symbol, get_next_label());
  }
  SEMICOLON
;

/* Declaração de subrotinas */
subroutine_declaration_part :
  { subroutine_label = get_next_label();
    generate_code(NULL, "DSVS %s", get_label_string(subroutine_label));
    uipush(&subroutine_stack, subroutine_label); }
  declare_subroutines_block
  { generate_label(uipop(&subroutine_stack)); }
;

subroutine_ident :
  IDENT {
    ++lexical_level;
    subroutine_label = get_next_label();
    subroutine_ptr = create_symbol(token, subroutine_feature, subroutine_label);
    generate_code(get_label_string(subroutine_label), "ENPR %u", lexical_level);
  }
;

subroutine_declaration :
  subroutine_ident PARENTHESES_OPEN parameter_declaration_part PARENTHESES_CLOSE {
    subroutine_ptr->sym_nparams = block_parameters;
    subroutine_ptr->sym_offset = -(block_parameters + 4); 
    push(&subroutine_stack, subroutine_ptr);
  }
  | /* OR */
  subroutine_ident {
    subroutine_ptr->sym_nparams = 0;
    push(&subroutine_stack, subroutine_ptr);
  }
;

subroutine_body :
  SEMICOLON block {
    subroutine_ptr = pop(&subroutine_stack);
    generate_code(NULL, "RTPR %u, %u", lexical_level, subroutine_ptr->sym_nparams);
    --lexical_level;
  }
  SEMICOLON
;

declare_subroutines_block :
  FUNCTION {
    subroutine_feature = function_symbol;
  }
  subroutine_declaration COLON type {
    subroutine_ptr->sym_type = symbol_type_id;
  }
  subroutine_body
  declare_subroutines_block
  | /* OR */
  PROCEDURE {
    subroutine_feature = procedure_symbol;
  }
  subroutine_declaration {
    subroutine_ptr->sym_type = sym_type_null;
  }
  subroutine_body
  declare_subroutines_block
  | /* OR */
  /* Nothing */
;

/* Declaração de parametros */
parameter_declaration_part : {
    block_parameters = 0;
  }
  parameter_block {
    set_parameters_offset(block_parameters);
    param_feature = val_parameter_symbol;
  }
  | /* OR */
  /* Nothing */
;

parameter_type_declaration :
  VAR {
    param_feature = ref_parameter_symbol;
  }
  | /* OR */
  /* Nothing */
  { param_feature = val_parameter_symbol; }
;

parameter_block :
  parameter_type_declaration parameter_line {
    block_parameters += line_parameters;
  }
  SEMICOLON parameter_block
  | /* OR */
  parameter_type_declaration parameter_line {
    block_parameters += line_parameters;
  }
;

parameter_line : {
    line_parameters = 0;
  }
  parameter_list COLON type {
    set_last_symbols_type(line_parameters, symbol_type_id);
    insert_params(&(subroutine_ptr->sym_params), line_parameters, symbol_type_id, param_feature);
  }
;

parameter_list :
  IDENT {
    create_symbol(token, param_feature, 0);
    ++line_parameters;
  }
  | /* OR */
  parameter_list COMMA IDENT {
    create_symbol(token, param_feature, 0);
    ++line_parameters;
  }
;

/* Declaração de variáveis */
variable_declaration_part :
  { block_variables = 0; }
  VAR variable_block
  | /* OR */
  /* Nothing */
;

variable_block :
  variable_line variable_block
  | /* OR */
  variable_line
;

variable_line : {
    line_variables = 0;
  }
  variable_list COLON type  {
    set_last_symbols_type(line_variables, symbol_type_id);
    generate_code(NULL, "AMEM %u", line_variables);
  }
  SEMICOLON
;

variable_list :
  variable_list COMMA IDENT {
    create_symbol(token, variable_symbol, 0);
    ++line_variables;
  }
  | /* OR */
  IDENT {
    create_symbol(token, variable_symbol, 0);
    ++line_variables;
  }
;

type :
  IDENT {
    symbol_type_id = parse_type(token);
  }
;

ident_list :
  ident_list COMMA IDENT
  | /* OR */ 
  IDENT
;

variable :
  IDENT {
    if((variable_ptr = find_variable_or_parameter(token)) == NULL) {
      variable_ptr = find_symbol(token, function_symbol, 1);
    }
  }
;

/* Comandos */
simple_or_composed_instructions :
  instruction
  | /* OR */
  composed_instructions
;

composed_instructions :
  T_BEGIN instructions T_END 
;

instructions :
  instruction
  | /* OR */
  instruction SEMICOLON instructions
  | /* OR */
  /* Nothing */
; 

instruction :
  set_instruction
  | /* OR */
  procedure_call
  | /* OR */
  conditional_instruction
  | /* OR */
  loop_instruction
  | /* OR */
  goto_instruction
  | /* OR */
  label_entry
  | /* OR */
  write_instruction
  | /* OR */
  read_instruction
;

set_instruction :
  variable {
    push(&variable_stack, variable_ptr);
  }
  SET expression {
    variable_ptr = pop(&variable_stack);
    process_stack_type(&expr_stack, variable_ptr->sym_type, NULL);

    if(variable_ptr->sym_feature == ref_parameter_symbol) {
      generate_code(NULL, "ARMI %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    } else {
      generate_code(NULL, "ARMZ %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    }
  }
;

procedure_call :
  IDENT {
    subroutine_ptr = find_symbol(token, procedure_symbol, 1);
  }
  PARENTHESES_OPEN expression_list PARENTHESES_CLOSE {
    generate_code(NULL, "CHPR %s, %d", get_label_string(subroutine_ptr->sym_label), lexical_level);
  }
  | /* OR */
  IDENT {
    subroutine_ptr = find_symbol(token, procedure_symbol, 1);
    generate_code(NULL, "CHPR %s, %d", get_label_string(subroutine_ptr->sym_label), lexical_level);
  }
;

function_call :
  IDENT {
    subroutine_ptr = find_symbol(token, function_symbol, 1);
    generate_code(NULL, "AMEM 1");
    push(&subroutine_stack, subroutine_ptr);
  }
  PARENTHESES_OPEN expression_list PARENTHESES_CLOSE {
    subroutine_ptr = pop(&subroutine_stack);
    generate_code(NULL, "CHPR %s, %d", get_label_string(subroutine_ptr->sym_label), lexical_level);
  }
;

if_statement :
  IF expression {
    if_label = get_next_label();
    if_not_label = get_next_label();
    process_stack_type(&expr_stack, sym_type_boolean, NULL);
    generate_code(NULL, "DSVF %s", get_label_string(if_not_label));
    uipush(&if_stack, if_not_label);
    uipush(&if_stack, if_label);
  }
  THEN simple_or_composed_instructions
;

conditional_instruction :
  if_statement %prec LOWER_THAN_ELSE {
    if_label = uipop(&if_stack);
    generate_code(NULL, "DSVS %s", get_label_string(if_label));
    generate_label(uipop(&if_stack));
    generate_label(if_label);
  }
  | /* OR */
  if_statement ELSE {
    if_label = uipop(&if_stack);
    generate_code(NULL, "DSVS %s", get_label_string(if_label));
    generate_label(uipop(&if_stack));
    uipush(&if_stack, if_label);
  }
  simple_or_composed_instructions {
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
  DO simple_or_composed_instructions {
    while_outter_label = uipop(&while_stack);
    while_inner_label = uipop(&while_stack);
    generate_code(NULL, "DSVS %s", get_label_string(while_inner_label));
    generate_label(while_outter_label);
  }
;

goto_instruction :
  GOTO CONSTANT {
    generate_code(NULL, "DSVR %s, %u, %u", get_label_string(find_symbol(token, label_symbol, 1)->sym_label), find_symbol(token, label_symbol, 1)->sym_lex_level, lexical_level);
  }
;

label_entry :
  CONSTANT {
    generate_code(get_label_string(find_symbol(token, label_symbol, 1)->sym_label), "ENRT %u %u", lexical_level, (subroutine_ptr != NULL) ? subroutine_ptr->sym_nparams : 0);
  }
  COLON instruction
;

write_instruction :
  WRITE PARENTHESES_OPEN write_expression_list PARENTHESES_CLOSE
;

write_expression_list :
  expression {
    generate_code(NULL, "IMPR");
  }
  COMMA write_expression_list
  | /* OR */
  expression {
    generate_code(NULL, "IMPR");
  }
;

read_instruction :
  READ PARENTHESES_OPEN read_variable_list PARENTHESES_CLOSE
;

read_variable_list :
  variable {
    generate_code(NULL, "LEIT");

    if(variable_ptr->sym_feature == ref_parameter_symbol) {
      generate_code(NULL, "ARMI %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    } else {
      generate_code(NULL, "ARMZ %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    }
  }
  COMMA read_variable_list
  | /* OR */
  variable {
    generate_code(NULL, "LEIT");

    if(variable_ptr->sym_feature == ref_parameter_symbol) {
      generate_code(NULL, "ARMI %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    } else {
      generate_code(NULL, "ARMZ %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
    }
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
  simple_expression {
    process_stack_type(&expr_stack, sym_type_integer, NULL);
    process_stack_type(&expr_stack, sym_type_integer, NULL);

    if(relation == sym_equal) {
      generate_code(NULL, "CMIG");
    } else if(relation == sym_diff) {
      generate_code(NULL, "CMDG");
    } else if(relation == sym_less_than) {
      generate_code(NULL, "CMME");
    } else if(relation == sym_less_or_equal_than) {
      generate_code(NULL, "CMEG");
    } else if(relation == sym_higher_than) {
      generate_code(NULL, "CMMA");
    } else if(relation == sym_higher_or_equal_than) {
      generate_code(NULL, "CMAG");
    }

    ipush(&expr_stack, (int) sym_type_boolean);
  }
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
    generate_code(NULL, "SUBT");
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
  term DIV_INT factor {
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
    ipush(&factor_stack, (int) variable_ptr->sym_type);

    if(variable_ptr->sym_feature != ref_parameter_symbol) {
      if(param_feature == ref_parameter_symbol) {
        generate_code(NULL, "CREN %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
      } else {
        generate_code(NULL, "CRVL %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
      }
    } else {
      if(param_feature == ref_parameter_symbol) {
        generate_code(NULL, "CRVL %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
      } else {
        generate_code(NULL, "CRVI %u, %d", variable_ptr->sym_lex_level, variable_ptr->sym_offset);
      }
    }
  }
  | /* OR */
  CONSTANT {
    ipush(&factor_stack, (int) sym_type_integer);
    generate_code(NULL, "CRCT %s", token);
  }
  | /* OR */
  function_call {
    ipush(&factor_stack, (int) subroutine_ptr->sym_type);
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
  PARENTHESES_OPEN expression PARENTHESES_CLOSE {
    transfer_stack_type(&expr_stack, &factor_stack);
  }
;

expression_list : { 
    line_parameters = 0;
    param_feature = get_param_feature(subroutine_ptr->sym_params, line_parameters);
  }
  expression {
    check_param(subroutine_ptr->sym_params, line_parameters++, ipop(&expr_stack));
    param_feature = val_parameter_symbol;
  }
  expr_comma_separated
  | /* OR */
  /* Nothing */
;

expr_comma_separated :
  COMMA {
    param_feature = get_param_feature(subroutine_ptr->sym_params, line_parameters);
  }
  expression {
    check_param(subroutine_ptr->sym_params, line_parameters++, ipop(&expr_stack));
    param_feature = val_parameter_symbol;
  }
  expr_comma_separated
  | /* OR */
  /* Nothing */
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

