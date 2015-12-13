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

  fprintf(stderr, "Line %d: ", line_number);
  vfprintf(stderr, error, args);
  exit(-1);
}

symbol_type parse_type(const char *type) {
  return  (strcasecmp(type, "integer") == 0) ? sym_type_integer :
          (strcasecmp(type, "boolean") == 0) ? sym_type_boolean : sym_type_null;
}

char *get_symbol_type_string(symbol_type type) {
  static char int_type_str[]  = "integer",
              bool_type_str[] = "boolean",
              null_type_str[] = "null";

  return  (type == sym_type_integer) ? int_type_str :
          (type == sym_type_boolean) ? bool_type_str : null_type_str;
}

char *get_symbol_feature_string(symbol_feature feature) {
  static char variable_str[]  = "variable",
              function_str[] = "function",
              procedure_str[] = "procedure",
              parameter_str[] = "parameter",
              null_str[] = "null";

  return  (feature == variable_symbol) ? variable_str :
          (feature == function_symbol) ? function_str :
          (feature == procedure_symbol) ? procedure_str :
          (feature == ref_parameter_symbol) ? parameter_str :
          (feature == val_parameter_symbol) ? parameter_str : null_str;
}

struct symbol_table *create_symbol(const char *name, symbol_feature feature, unsigned int label) {
  struct symbol_table *sym, *chksym;

  if((chksym = find_symbol(name, null_symbol, 0)) != NULL && chksym->sym_lex_level == lexical_level) {
    print_error("Symbol \"%s\" already defined on this lexical level!", name);
  }

  sym = (struct symbol_table *) malloc(sizeof(struct symbol_table));
  sym->sym_name = strdup(name);
  sym->sym_feature = feature;
  sym->sym_type = sym_type_null;
  sym->sym_lex_level = lexical_level;
  sym->sym_label = label;
  sym->sym_nparams = 0;
  sym->sym_params = NULL;

  if(sym == NULL || sym->sym_name == NULL) {
    if(sym != NULL) {
      free(sym);
    }

    fprintf(stderr, "create_symbol(): Cannot allocate memory for symbol \"%s\".\n", name);
    return NULL;
  }

  if(sym_tb_base != NULL) {
    sym->sym_next = sym_tb_base;
  }

  sym_tb_base = sym;

  if(feature == variable_symbol) {
    sym->sym_offset = block_variables++;
  } 

  return sym;
}

struct symbol_table *find_symbol(const char *name, symbol_feature feature, int must_exist) {
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0) {
      if(must_exist != 0 && feature != null_symbol && sym->sym_feature != feature) {
        print_error("Symbol \"%s\" is not a %s.", get_symbol_feature_string(feature)); 
      }

      return sym;
    }
  }

  if(must_exist != 0) {
    print_error("Undefined symbol \"%s\".\n", name);
  }

  return NULL;
}

struct symbol_table *find_variable_or_parameter(const char *name) {
  struct symbol_table *sym;

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0 && (sym->sym_feature == variable_symbol || sym->sym_feature == val_parameter_symbol || sym->sym_feature == ref_parameter_symbol)) {
      return sym;
    }
  }

  return NULL;
}

void set_last_symbols_type(unsigned int nsymbols, symbol_type tsym) {
  struct symbol_table *sym;
  unsigned int i;

  for(i = 0, sym = sym_tb_base; i < nsymbols && sym != NULL; ++i, sym = sym->sym_next) {
    sym->sym_type = tsym;
  }
}

void set_parameters_offset(unsigned int nsymbols) {
  struct symbol_table *sym;
  unsigned int i;
  int offset = -4;

  for(i = 0, sym = sym_tb_base; i < nsymbols && sym != NULL; ++i, sym = sym->sym_next) {
    sym->sym_offset = offset;
    --offset;
  }
}

void print_symbols_table() {
  struct symbol_table *sym;
  struct param_list *p;

  fprintf(stdout, "\n\nLEX - OFF - FEATURE  - TYPE - PARAMS - NAME\n");

  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    fprintf(stdout, "%3u - %3d - %s - %s ",
                                      sym->sym_lex_level,
                                      sym->sym_offset,
                                      get_symbol_feature_string(sym->sym_feature),
                                      get_symbol_type_string(sym->sym_type));

    if(sym->sym_feature == function_symbol || sym->sym_feature == procedure_symbol) {
      for(p = sym->sym_params; p != NULL; p = p->param_next) {
        fprintf(stdout, "%u%s ", p->param_count, (p->param_feature == val_parameter_symbol) ? "val" : "ref");
      }
    }

    fprintf(stdout, "- %s\n", sym->sym_name);
  }

  fprintf(stdout, "\n");
}

void free_level_symbols(unsigned int level) {
  struct symbol_table *sym, *free_sym, *prev_sym;
  unsigned int dmem_count = 0;

  prev_sym = NULL;
  sym = sym_tb_base;

  while(sym != NULL) {
    free_sym = NULL;

    switch(sym->sym_feature) {
      case variable_symbol:
        if(sym->sym_lex_level >= level) {
          free_sym = sym;
          ++dmem_count;
        }

        break;

      case val_parameter_symbol:
      case ref_parameter_symbol:
        if(sym->sym_lex_level >= level) {
          free_sym = sym;
        }

        break;

      case procedure_symbol:
      case function_symbol:
        if(sym->sym_lex_level > level) {
          free_sym = sym;
        }
    }

    if(free_sym == NULL) {
      prev_sym = sym;
    }

    sym = sym->sym_next;

    if(free_sym != NULL) {
      if(prev_sym != NULL) {
        prev_sym->sym_next = sym;
      } else {
        sym_tb_base = sym;
      }

      if(free_sym->sym_name != NULL) {
        free(free_sym->sym_name);
      }

      free(free_sym);
    }
  }

  if(dmem_count > 0) {
    generate_code(NULL, "DMEM %u", dmem_count);
  }
}

void free_symbols() {
  struct symbol_table *sym, *aux_sym;
  struct param_list *prev_params;

  sym = sym_tb_base;

  while(sym != NULL) {
    aux_sym = sym;
    sym = sym->sym_next;

    if(aux_sym->sym_name != NULL) {
      free(aux_sym->sym_name);
    }

    while(aux_sym->sym_params != NULL) {
      prev_params = aux_sym->sym_params;
      aux_sym->sym_params = aux_sym->sym_params->param_next;
      free(prev_params);
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

void uipush(struct stack_node **stack, unsigned int value) {
  unsigned int *uiptr;

  uiptr = (unsigned int *) malloc(sizeof(unsigned int));
  *uiptr = value;
  push(stack, uiptr);
}

unsigned int uipop(struct stack_node **stack) {
  unsigned int *uiptr;
  unsigned int ret;

  uiptr = pop(stack);
  ret = *uiptr;

  free(uiptr);
  return ret;
}

void process_stack_type(struct stack_node **stack, symbol_type type, struct stack_node **dest) {
  symbol_type tstk;

  tstk = (symbol_type) ipop(stack);

  if(tstk != type) {
    print_error("Invalid expression type, expected '%s', found '%s'\n", get_symbol_type_string(type), get_symbol_type_string(tstk));
  }

  if(dest != NULL) {
    ipush(dest, (int) tstk);
  }
}

void transfer_stack_type(struct stack_node **source, struct stack_node **dest) {
  ipush(dest, ipop(source));
}

void insert_params(struct param_list **dest, unsigned int nparams, symbol_type type, symbol_feature feature) {
  struct param_list *p, *last;
  unsigned int i;

  p = (struct param_list *) malloc(sizeof(struct param_list));

  if(p != NULL) {
    p->param_feature = feature;
    p->param_type = type;
    p->param_count = nparams;
    p->param_next = NULL;

    if(*dest == NULL) {
      *dest = p;
    } else {
      for(last = *dest; last->param_next != NULL; last = last->param_next);
      last->param_next = p;
    }
  }
}

symbol_feature get_param_feature(struct param_list *p, unsigned int param_no) {
  unsigned int i;

  for(i = 0; i + p->param_count <= param_no; i += p->param_count) {
    if(p->param_next == NULL) {
      print_error("Unexpected parameter for function (%d).\n", param_no);
    }

    p = p->param_next;
  }

  return p->param_feature;
}

void check_param(struct param_list *p, unsigned int param_no, symbol_type type) {
  unsigned int i;

  for(i = 0; i + p->param_count <= param_no; i += p->param_count) {
    if(p->param_next == NULL) {
      print_error("Unexpected parameter for function (%d).\n", param_no);
    }

    p = p->param_next;
  }

  if(p->param_type != type) {
    print_error("Invalid parameter %d type, expected '%s', found '%s'.\n", param_no, get_symbol_type_string(p->param_type), get_symbol_type_string(type));
  }
}

unsigned int get_next_label() {
  static unsigned int current_label = 0;
  return current_label++;
}

void generate_label(unsigned int label) {
  char label_str[MAX_LABEL];
  snprintf(label_str, sizeof label_str, "R%s%u", (label < 10) ? "0" : "", label);
  generate_code(label_str, "NADA");
}

char *get_label_string(unsigned int label) {
  static char label_str[MAX_LABEL];
  snprintf(label_str, sizeof label_str, "R%s%u", (label < 10) ? "0" : "", label);
  return label_str;
}
