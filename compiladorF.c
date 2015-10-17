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

    fprintf(stderr, "create_symbol(): Erro ao alocar memória para símbolo \"%s\".\n", name);
    return;
  }

  if(sym_tb_base != NULL) {
    sym->sym_next = sym_tb_base;
  }

  sym_tb_base = sym;
  ++block_variables;
}

char *get_symbol_reference(const char *name) {
  static char ref[MAX_SYMBOL_REF];
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0) {
      if(sym->sym_type == sym_type_var) {
        snprintf(ref, sizeof ref, "%u %u", sym->sym_lex_level, sym->sym_offset);
        return ref;
      }
    }
  }

  print_error("Simbolo indefinido: \"%s\".\n", name);

  /* Nunca executado */
  snprintf(ref, sizeof ref, "<null>");
  return ref;
}

unsigned int free_level_symbols() {
  struct symbol_table *sym, *aux_sym;
  unsigned int var_count = 0;

  sym = sym_tb_base;

  while(sym != NULL && sym->sym_lex_level == lexical_level) {
    if(sym->sym_type == sym_type_var) {
      ++var_count;
    }

    aux_sym = sym;
    sym = sym->sym_next;

    if(aux_sym->sym_name != NULL) {
      free(aux_sym->sym_name);
    }

    free(aux_sym);
  }

  sym_tb_base = sym;
  return var_count;
}

void free_symbols() {
  struct symbol_table *sym, *aux_sym;

  sym = sym_tb_base;

  while(sym != NULL) {
    aux_sym = sym;
    sym = sym->sym_next;

    if(aux_sym->sym_name != NULL) {
      free(aux_sym->sym_name);
    }

    free(aux_sym);
  }

  sym_tb_base = NULL;
}

void push(struct stack_node **stack, void *value) {
  struct stack_node *next;

  next = *stack;

  *stack = (struct stack_node *) malloc(sizeof(struct stack_node));

  if(*stack != NULL) {
    (*stack)->stack_next = next;
    (*stack)->stack_value = value;
  }
}

void *pop(struct stack_node **stack) {
  void *ret;
  struct stack_node *aux;

  if(*stack == NULL) {
    return NULL;
  }

  ret = (*stack)->stack_value;
  aux = *stack;
  
  *stack = (*stack)->stack_next;
  free(aux);
  return ret;
}

void ipush(struct stack_node **stack, int value) {
  int *iptr;

  iptr = (int *) malloc(sizeof(int));
  *iptr = value;
  push(stack, iptr);
}

int ipop(struct stack_node **stack) {
  int *iptr;
  int ret;

  iptr = pop(stack);
  ret = *iptr;

  free(iptr);
  return ret;
}
