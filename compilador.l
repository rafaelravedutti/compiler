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
identifier [a-zA-Z][a-zA-Z1-9]*
number [0-9]+
comment "(*".*"*)"

%%

{new_line} {
  ++line_number;
} 

{ignore}

{comment}

program {
  process_symbol(sym_program, PROGRAM);
}

var {
  process_symbol(sym_var, VAR);
}

begin {
  process_symbol(sym_begin, T_BEGIN);
}

end {
  process_symbol(sym_end, T_END);
}

while {
  process_symbol(sym_while, WHILE);
}

do {
  process_symbol(sym_do, DO);
}

for {
  process_symbol(sym_for, FOR);
}

to {
  process_symbol(sym_to, TO);
}

downto {
  process_symbol(sym_downto, DOWNTO);
}

if {
  process_symbol(sym_if, IF);
}

then {
  process_symbol(sym_then, THEN);
}

else {
  process_symbol(sym_else, ELSE);
}

procedure {
  process_symbol(sym_procedure, PROCEDURE);
}

function {
  process_symbol(sym_function, FUNCTION);
}

repeat {
  process_symbol(sym_repeat, REPEAT);
}

until {
  process_symbol(sym_until, UNTIL);
}

goto {
  process_symbol(sym_goto, GOTO);
}

label {
  process_symbol(sym_label, LABEL);
}

not {
  process_symbol(sym_not, NOT);
}

case {
  process_symbol(sym_case, CASE);
}

in {
  process_symbol(sym_in, IN);
}

true {
  process_symbol(sym_true, TRUE);
}

false {
  process_symbol(sym_false, FALSE);
}

:= {
  process_symbol(sym_set, SET);
}

\; {
  process_symbol(sym_semicolon, SEMICOLON);
}

\: {
  process_symbol(sym_colon, COLON);
}

\, {
  process_symbol(sym_comma, COMMA);
}

\. {
  process_symbol(sym_dot, DOT);
}

\( {
  process_symbol(sym_parentheses_open, PARENTHESES_OPEN);
}

\) {
  process_symbol(sym_parentheses_close, PARENTHESES_CLOSE);
}

= {
  process_symbol(sym_equal, EQUAL);
}

\<\> {
  process_symbol(sym_diff, DIFF);
}

\< {
  process_symbol(sym_less_than, LESS_THAN);
}

\> {
  process_symbol(sym_higher_than, HIGHER_THAN);
}

\<= {
  process_symbol(sym_less_or_equal_than, LESS_OR_EQUAL_THAN);
}

\>= {
  process_symbol(sym_higher_or_equal_than, HIGHER_OR_EQUAL_THAN);
}

and {
  process_symbol(sym_and, AND);
}

or {
  process_symbol(sym_or, OR);
}

\+ {
  process_symbol(sym_sum, SUM);
}

\- {
  process_symbol(sym_sub, SUB);
}

\* {
  process_symbol(sym_times, TIMES);
}

\/ {
  process_symbol(sym_div, DIV);
}

\% {
  process_symbol(sym_mod, MOD);
}

div {
  process_symbol(sym_div_int, DIV_INT);
}

write {
  process_symbol(sym_write, WRITE);
}

read {
  process_symbol(sym_read, READ);
}

{identifier} {
  process_token(sym_identifier, IDENT);
}

{number} {
  process_token(sym_number, CONSTANT);
}

%%

