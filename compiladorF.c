#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "compilador.h"

static FILE *fp = NULL;
static struct symbol_table *sym_tb_base = NULL;

void generate_code(const char *label, const char *code, ...) {
  va_list args;

  va_start(args, code);

  if(fp == NULL) {
    fp = fopen ("MEPA", "w");
  }

  if(label != NULL) {
    fprintf(fp, "%s: ", label);
  } else {
    fprintf(fp, "     ");
  }

  vfprintf(fp, code, args);
  fprintf(fp, "\n");
  fflush(fp);
}

void print_error(const char *error, ...) {
  va_list args;

  va_start(args, error);

  fprintf(stderr, "Linha %d: ", line_number);
  vfprintf(stderr, error, args);
  exit(-1);
}

void create_symbol(const char *name, symbol_type type) {
  struct symbol_table *sym;

  sym = (struct symbol_table *) malloc(sizeof(struct symbol_table));
  sym->sym_name = strdup(name);
  sym->sym_type = type;
  sym->sym_lex_level = lexical_level;
  sym->sym_offset = block_variables;

  if(sym == NULL || sym->sym_name == NULL) {
    if(sym != NULL) {
      free(sym);
    }

    fprintf(stderr, "insertSymbol(): Erro ao alocar memória para símbolo \"%s\".\n", name);
    return;
  }

  if(sym_tb_base != NULL) {
    sym_tb_base->sym_next = sym;
  } else {
    sym_tb_base = sym;
  }

  ++block_variables;
}

char *get_symbol_ref(const char *name) {
  static char ref[MAX_SYMBOL_REF];
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0) {
      if(sym->sym_type == sym_type_var) {
        snprintf(ref, sizeof ref, "%d %d", sym->sym_lex_level, sym->sym_offset);
        return ref;
      }
    }
  }

  print_error("Simbolo indefinido: \"%s\".\n", name);

  /* Nunca executado */
  snprintf(ref, sizeof ref, "<null>");
  return ref;
}

void free_level_symbols() {
  struct symbol_table *sym, *aux_sym;
  unsigned int lex_level;

  sym = sym_tb_base;
  lex_level = sym_tb_base->sym_lex_level;

  while(sym != NULL && sym->sym_lex_level == lex_level) {
    aux_sym = sym;
    sym = sym->sym_next;
    free(aux_sym);
  }

  sym_tb_base = sym;
}

void free_symbols() {
  struct symbol_table *sym, *aux_sym;

  sym = sym_tb_base;

  while(sym != NULL) {
    aux_sym = sym;
    sym = sym->sym_next;
    free(aux_sym);
  }

  sym_tb_base = NULL;
}
