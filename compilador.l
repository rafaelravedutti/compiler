%{


#include<stdio.h>
#include<string.h>

#include "compilador.h"
#include "compilador.tab.h"

#define DEBUG

#define _process_token(sym)           \
  symbol = sym;                       \
  strncpy(token, yytext, TAM_TOKEN)

#ifdef DEBUG
#  define process_token(sym,ret)      \
    _process_token(sym);              \
    fprintf(stdout, #sym " ");        \
    return ret
#else
#  define process_token(sym,ret)      \
    _process_token(sym);              \
    return ret
#endif

unsigned int line_number = 1;  

%}

new_line [\n]
ignore [\t ]+
identifier [a-zAZ][a-zAZ1-9]*
number [0-9]+
comment "(*".*"*)"

%%

{new_line} { ++line_number; } 

{ignore}

{comment}

program {
  process_token(sym_program, PROGRAM);
}

var {
  process_token(sym_var, VAR);
}

begin {
  process_token(sym_begin, T_BEGIN);
}

end {
  process_token(sym_end, T_END);
}

:= {
  process_token(sym_set, SET);
}

\; {
  process_token(sym_semicolon, SEMICOLON);
}

\: {
  process_token(sym_colon, COLON);
}

\, {
  process_token(sym_comma, COMMA);
}

\. {
  process_token(sym_dot, DOT);
}

\( {
  process_token(sym_parentheses_open, PARENTHESES_OPEN);
}

\) {
  process_token(sym_parentheses_close, PARENTHESES_CLOSE);
}

{identifier} {
  process_token(sym_identifier, IDENT);
}

%%

