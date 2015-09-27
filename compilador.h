/* -------------------------------------------------------------------
 *            Arquivo: compilaodr.h
 * -------------------------------------------------------------------
 *              Autor: Bruno Muller Junior
 *               Data: 08/2007
 *      Atualizado em: [15/03/2012, 08h:22m]
 *
 * -------------------------------------------------------------------
 *
 * Tipos, protótipos e vaiáveis globais do compilador
 *
 * ------------------------------------------------------------------- */

#define TAM_TOKEN 16

typedef enum symbol_names { 
  sym_program, sym_var, sym_begin, sym_end, 
  sym_identifier, sym_number,
  sym_dot, sym_comma, sym_semicolon, sym_colon,
  sym_set, sym_parentheses_open, sym_parentheses_close,
} symbol_name;



/* -------------------------------------------------------------------
 * variáveis globais
 * ------------------------------------------------------------------- */

extern symbol_name simbolo, relacao;
extern char token[TAM_TOKEN];
extern int nivel_lexico;
extern int desloc;
extern unsigned int line_number;


symbol_name symbol, relacao;
char token[TAM_TOKEN];

