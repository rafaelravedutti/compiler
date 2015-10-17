%{

#include <stdio.h>
#include <string.h>

#include "compilador.h"
#include "compilador.tab.h"

#define DEBUG

#define _process_token(sym)                 \
  symbol = sym;                             \
  strncpy(token, yytext, MAX_TOKEN)

#define _process_symbol(sym)                \
  symbol = sym
 
#ifdef DEBUG
#  define process_token(sym,ret)            \
    _process_token(sym);                    \
    fprintf(stdout, #sym "(%s) ", yytext);  \
    return ret

#  define process_symbol(sym,ret)           \
    _process_symbol(sym);                   \
    fprintf(stdout, #sym "(%s) ", yytext);  \
    return ret

#else
#  define process_token(sym,ret)            \
    _process_token(sym);                    \
    return ret

#  define process_symbol(sym,ret)           \
    _process_symbol(sym);                   \
    return ret
#endif

unsigned int lexical_level = 0;
unsigned int line_number = 1;  

%}

new_line [\n]
ignore [\t ]+
identifier [a-zAZ][a-zAZ1-9]*
number [0-9]+
comment "(*".*"*)"

%%

{new_line} {
  ++line_number;
} 

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

while {
  process_token(sym_while, WHILE);
}

do {
  process_token(sym_do, DO);
}

for {
  process_token(sym_for, FOR);
}

to {
  process_token(sym_to, TO);
}

downto {
  process_token(sym_downto, DOWNTO);
}

if {
  process_token(sym_if, IF);
}

then {
  process_token(sym_then, THEN);
}

else {
  process_token(sym_else, ELSE);
}

procedure {
  process_token(sym_procedure, PROCEDURE);
}

function {
  process_token(sym_function, FUNCTION);
}

repeat {
  process_token(sym_repeat, REPEAT);
}

until {
  process_token(sym_until, UNTIL);
}

goto {
  process_token(sym_goto, GOTO);
}

label {
  process_token(sym_label, LABEL);
}

not {
  process_token(sym_not, NOT);
}

case {
  process_token(sym_case, CASE);
}

in {
  process_token(sym_in, IN);
}

:= {
  process_symbol(sym_set, SET);
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
  process_symbol(sym_parentheses_open, PARENTHESES_OPEN);
}

\) {
  process_symbol(sym_parentheses_close, PARENTHESES_CLOSE);
}

= {
  process_token(sym_equal, EQUAL);
}

\<\> {
  process_token(sym_diff, DIFF);
}

\< {
  process_token(sym_less_than, LESS_THAN);
}

\> {
  process_token(sym_higher_than, HIGHER_THAN);
}

\<= {
  process_token(sym_less_or_equal_than, LESS_OR_EQUAL_THAN);
}

\>= {
  process_token(sym_higher_or_equal_than, HIGHER_OR_EQUAL_THAN);
}

and {
  process_token(sym_and, AND);
}

or {
  process_token(sym_or, OR);
}

\+ {
  process_token(sym_sum, SUM);
}

\- {
  process_token(sym_sub, SUB);
}

\* {
  process_token(sym_times, TIMES);
}

\/ {
  process_token(sym_div, DIV);
}

\% {
  process_token(sym_mod, MOD);
}

{identifier} {
  process_token(sym_identifier, IDENT);
}

{number} {
  process_token(sym_number, CONSTANT);
}

%%

