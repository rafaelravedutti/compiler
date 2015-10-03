%{
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"

%}

%token PROGRAM PARENTHESES_OPEN PARENTHESES_CLOSE 
%token COMMA SEMICOLON COLON DOT
%token T_BEGIN T_END VAR IDENT SET

%%

programa :  
  { generate_code(NULL, "INPP"); }
  PROGRAM IDENT 
  PARENTHESES_OPEN lista_idents PARENTHESES_CLOSE SEMICOLON
  bloco DOT
  { generate_code(NULL, "PARA"); }
;

bloco :
  parte_declara_vars
  { }
  comando_composto 
;




parte_declara_vars:
  var 
;

var :
  { }
  VAR declara_vars |
;

declara_vars :
  declara_vars declara_var | declara_var 
;

declara_var :
  { } 
  lista_id_var COLON
  tipo 
  { /* AMEM */ }
  SEMICOLON
;

tipo :
  IDENT
;

lista_id_var :
  lista_id_var COMMA IDENT { /* insere última vars na tabela de símbolos */ } | IDENT { /* insere vars na tabela de símbolos */}
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

  fp = fopen(argv[1], "r");
  if(fp == NULL) {
    fprintf(stdout, "Ocorreu um erro ao abrir o arquivo!\n");
    return(-1);
  }

  yyin = fp;
  yyparse();
  return 0;
}

