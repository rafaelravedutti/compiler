#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"

static FILE *fp = NULL;
static struct symbol_table *sym_tb_base = NULL;

void generate_code(const char *label, const char *code) {
  if(fp == NULL) {
    fp = fopen ("MEPA", "w");
  }

  if(label == NULL) {
    fprintf(fp, "     %s\n", code);
  } else {
    fprintf(fp, "%s: %s \n", label, code);
  }

  fflush(fp);
}

void print_error(const char *error) {
  fprintf(stderr, "Erro na linha %d - %s\n", line_number, error);
  exit(-1);
}

void insert_symbol(const char *name, symbol_type type) {
  struct symbol_table *sym;

  sym = (struct symbol_table *) malloc(sizeof(struct symbol_table));
  sym->sym_name = strdup(name);
  sym->sym_type = type;
  sym->sym_lex_level = lexical_level;
  sym->sym_offset = var_offset;

  if(sym == NULL || sym->sym_name == NULL) {
    if(sym != NULL) {
      free(sym);
    }

    fprintf(stderr, "insertSymbol(): Erro ao alocar memória para símbolo \"%s\".", name);
    return;
  }

  if(sym_tb_base != NULL) {
    sym_tb_base->next = sym;
  }

  sym_tb_base = sym;
}

const char *get_symbol_ref(const char *name) {
  static char ref[MAX_SYMBOL_REF];
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->next) {
    if(strcmp(sym->sym_name, name) == 0) {
      if(sym->sym_type == SYM_VAR) {
        sprintf(ref, "%d,%d", sym->sym_lex_level, sym->sym_offset);
        return ref;
      }
    }
  }

  fprintf(stderr, "Símbolo indefinido: \"%s\".", name);
  exit(-1);

  /* Nunca executado */
  sprintf(ref, "<null>");
  return ref;
}
