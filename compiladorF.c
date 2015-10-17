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

symbol_type parse_type(const char *type) {
  return  (strcasecmp(type, "integer") == 0) ? sym_type_integer :
          (strcasecmp(type, "boolean") == 0) ? sym_type_boolean :
          sym_type_null;
}

struct symbol_table *create_symbol(const char *name, symbol_feature feature) {
  struct symbol_table *sym, *chksym;

  if((chksym = find_symbol(name)) != NULL && chksym->sym_lex_level == lexical_level) {
    print_error("Simbolo \"%s\" ja definido nesse nivel lexico!", name);
  }

  sym = (struct symbol_table *) malloc(sizeof(struct symbol_table));
  sym->sym_name = strdup(name);
  sym->sym_feature = feature;
  sym->sym_type = sym_type_null;
  sym->sym_lex_level = lexical_level;
  sym->sym_offset = block_variables;

  if(sym == NULL || sym->sym_name == NULL) {
    if(sym != NULL) {
      free(sym);
    }

    fprintf(stderr, "create_symbol(): Erro ao alocar memoria para simbolo \"%s\".\n", name);
    return NULL;
  }

  if(sym_tb_base != NULL) {
    sym->sym_next = sym_tb_base;
  }

  sym_tb_base = sym;
  ++block_variables;
  return sym;
}

struct symbol_table *find_symbol(const char *name) {
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0) {
      return sym;
    }
  }

  return NULL;
}

char *get_symbol_reference(const char *name) {
  static char ref[MAX_SYMBOL_REF];
  struct symbol_table *sym;

  sym = find_symbol(name);

  if(sym != NULL) {
    if(sym->sym_feature == variable_symbol) {
      snprintf(ref, sizeof ref, "%u %u", sym->sym_lex_level, sym->sym_offset);
      return ref;
    }
  }

  print_error("Simbolo indefinido: \"%s\".\n", name);

  /* Nunca executado */
  snprintf(ref, sizeof ref, "<null>");
  return ref;
}

void set_last_symbols_type(unsigned int nsymbols, symbol_type tsym) {
  struct symbol_table *sym;
  unsigned int i;

  for(i = 0, sym = sym_tb_base; i < nsymbols && sym != NULL; ++i, sym = sym->sym_next) {
    sym->sym_type = tsym;
  }
}

void print_symbols_table() {
  struct symbol_table *sym;

  fprintf(stdout, "\n\nLEX - OFF - FEATURE  - TYPE - NAME\n");

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    fprintf(stdout, "%3u - %3u - ", sym->sym_lex_level, sym->sym_offset);

    if(sym->sym_feature == function_symbol) {
      fprintf(stdout, "function - ");
    } else if(sym->sym_feature == variable_symbol) {
      fprintf(stdout, "variable - ");
    } else {
      fprintf(stdout, "  null   - ");
    }

    if(sym->sym_type == sym_type_integer) {
      fprintf(stdout, "INT  - ");
    } else if(sym->sym_type == sym_type_boolean) {
      fprintf(stdout, "BOOL - ");
    } else {
      fprintf(stdout, "NULL - ");
    }

    fprintf(stdout, "%s\n", sym->sym_name);
  }

  fprintf(stdout, "\n");
}

unsigned int free_level_symbols() {
  struct symbol_table *sym, *aux_sym;
  unsigned int var_count = 0;

  sym = sym_tb_base;

  while(sym != NULL && sym->sym_lex_level == lexical_level) {
    if(sym->sym_feature == variable_symbol) {
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
