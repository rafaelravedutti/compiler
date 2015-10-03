%{
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"

%}

%token PROGRAM PARENTHESES_OPEN PARENTHESES_CLOSE 
%token COMMA SEMICOLON COLON DOT VAR IDENT SET
%token T_BEGIN T_END WHILE FOR TO DOWNTO IF THEN ELSE
%token PROCEDURE FUNCTION REPEAT UNTIL GOTO LABEL NOT
%token CASE IN

%%

program :  
  { generate_code(NULL, "INPP"); }
  PROGRAM IDENT PARENTHESES_OPEN lista_idents PARENTHESES_CLOSE SEMICOLON block DOT
  { generate_code(NULL, "PARA"); }
;

block :
  variable_declaration_part
  { }
  comando_composto 
;

variable_declaration_part :
  { }
  VAR declare_vars_block |
;

declare_vars_block :
  declare_vars_block declare_vars_line |
  declare_vars_line
;

declare_vars_line :
  { declared_variables = 0; }
  variable_list COLON
  tipo 
  { generate_code(NULL, "AMEM %d", declared_variables); }
  SEMICOLON
;

tipo :
  IDENT
;

variable_list :
  variable_list COMMA IDENT { create_symbol(token, sym_type_var), ++declared_variables; } | 
  IDENT { create_symbol(token, sym_type_var), ++declared_variables; }
;

lista_idents :
  lista_idents COMMA IDENT | IDENT
;


comando_composto:
  T_BEGIN comandos T_END 

comandos:    
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

