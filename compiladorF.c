#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "compilador.h"

static FILE *fp = NULL;
static struct symbol_table *sym_tb_base = NULL;

/* Gera código no arquivo MEPA de destino */
void generate_code(const char *label, const char *code, ...) {
  va_list args;

  /* Inicializa argumentos a partir do parâmetro "code" */
  va_start(args, code);

  /* Se ainda não foi definido o arquivo de destino do código, cria-o */
  if(fp == NULL) {
    fp = fopen ("MEPA", "w");
  }

  /* Se foi especificada um rótulo a ser gerado, escreve-o */
  if(label != NULL) {
    fprintf(fp, "%s: ", label);
  } else {
    fprintf(fp, "     ");
  }

  /* Formata, escreve o código e atualiza o arquivo destino */
  vfprintf(fp, code, args);
  fprintf(fp, "\n");
  fflush(fp);
}

/* Imprime um erro de compilação */
void print_error(const char *error, ...) {
  va_list args;

  /* Inicializa argumentos a partir do parâmetro "error" */
  va_start(args, error);

  /* Imprime a linha do erro, o erro e finaliza o compilador retornando erro */
  fprintf(stderr, "Line %d: ", line_number);
  vfprintf(stderr, error, args);
  exit(-1);
}

/* Dada uma string, retorna o valor que representa seu tipo */
symbol_type parse_type(const char *type) {
  return  (strcasecmp(type, "integer") == 0) ? sym_type_integer :
          (strcasecmp(type, "boolean") == 0) ? sym_type_boolean : sym_type_null;
}

/* Dado um tipo, retorna uma string que o representa */
char *get_symbol_type_string(symbol_type type) {
  static char int_type_str[]  = "integer",
              bool_type_str[] = "boolean",
              null_type_str[] = "null";

  return  (type == sym_type_integer) ? int_type_str :
          (type == sym_type_boolean) ? bool_type_str : null_type_str;
}

/* Dada uma caracteristica, retorna uma string que a representa */
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

/* Cria um símbolo na tabela de símbolos */
struct symbol_table *create_symbol(const char *name, symbol_feature feature, unsigned int label) {
  struct symbol_table *sym, *chksym;

  /* Verifica se o símbolo já existe no nível léxico atual e informa erro se existir */
  if((chksym = find_symbol(name, null_symbol, 0)) != NULL && chksym->sym_lex_level == lexical_level) {
    print_error("Symbol \"%s\" already defined on this lexical level!", name);
  }

  /* Aloca o nodo do símbolo */
  sym = (struct symbol_table *) malloc(sizeof(struct symbol_table));

  /* Se houve erro durante a alocação, informa-o e finaliza o compilador */
  if(sym == NULL) {
    fprintf(stderr, "create_symbol(): Cannot allocate memory for symbol \"%s\".\n", name);
    return NULL;
  }

  /* Define as informações do nodo */
  sym->sym_name = strdup(name);
  sym->sym_feature = feature;
  sym->sym_type = sym_type_null;
  sym->sym_lex_level = lexical_level;
  sym->sym_label = label;
  sym->sym_nparams = 0;
  sym->sym_params = NULL;

  /* Insere o nodo no inicio da tabela de símbolos */
  if(sym_tb_base != NULL) {
    sym->sym_next = sym_tb_base;
  }

  sym_tb_base = sym;

  /* Se o símbolo é uma variável, aumenta o contador de variáveis no bloco atual e define seu deslocamento */
  if(feature == variable_symbol) {
    sym->sym_offset = block_variables++;
  } 

  return sym;
}

/* Procura um símbolo na tabela de símbolos */
struct symbol_table *find_symbol(const char *name, symbol_feature feature, int must_exist) {
  struct symbol_table *sym;

  /* Percorre a tabela de símbolos até encontrar o símbolo com o nome informado */
  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0) {
      /* Se o símbolo deveria existir e não é do tipo especificado, retorna erro */
      if(must_exist != 0 && feature != null_symbol && sym->sym_feature != feature) {
        print_error("Symbol \"%s\" is not a %s.", get_symbol_feature_string(feature)); 
      }

      return sym;
    }
  }

  /* Se o símbolo não existe e deveria existir, retorna erro */
  if(must_exist != 0) {
    print_error("Undefined symbol \"%s\".\n", name);
  }

  return NULL;
}

/* Procura uma variável ou parâmetro na tabela de símbolos */
struct symbol_table *find_variable_or_parameter(const char *name) {
  struct symbol_table *sym;

  /* Percorre a tabela de símbolos até encontrar e retornar a variável/parâmetro com o nome informado */
  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    if(strcmp(sym->sym_name, name) == 0 && (sym->sym_feature == variable_symbol || sym->sym_feature == val_parameter_symbol || sym->sym_feature == ref_parameter_symbol)) {
      return sym;
    }
  }

  /* Retorna NULL se não existir */
  return NULL;
}

/* Define o tipo dos últimos símbolos adicionados na tabela de símbolos */
void set_last_symbols_type(unsigned int nsymbols, symbol_type tsym) {
  struct symbol_table *sym;
  unsigned int i;

  /* Percorre os últimos nsymbols adicionados e define seu tipo de dados para tsym */
  for(i = 0, sym = sym_tb_base; i < nsymbols && sym != NULL; ++i, sym = sym->sym_next) {
    sym->sym_type = tsym;
  }
}

/* Define o deslocamento dos últimos símbolos/parâmetros na tabela de símbolos */
void set_parameters_offset(unsigned int nsymbols) {
  struct symbol_table *sym;
  unsigned int i;
  int offset = -4;

  /* Percorre os últimos nsymbols definindo seus deslocamentos decrementando a partir do -4 (primeiro parâmetro) */
  for(i = 0, sym = sym_tb_base; i < nsymbols && sym != NULL; ++i, sym = sym->sym_next) {
    sym->sym_offset = offset;
    --offset;
  }
}

/* Imprime a tabela de símbolos */
void print_symbols_table() {
  struct symbol_table *sym;
  struct param_list *p;

  /* Imprime o cabeçalho */
  fprintf(stdout, "\n\nLEX - OFF - FEATURE  - TYPE - PARAMS - NAME\n");

  /* Percorre a tabela de símbolos */
  for(sym = sym_tb_base; sym != NULL; sym = sym->sym_next) {
    /* Imprime nivel lexico, deslocamento, característica e o tipo do símbolo */
    fprintf(stdout, "%3u - %3d - %s - %s ",
                                      sym->sym_lex_level,
                                      sym->sym_offset,
                                      get_symbol_feature_string(sym->sym_feature),
                                      get_symbol_type_string(sym->sym_type));

    /* Se for função ou procedimento, imprime os parâmetros especificados */
    if(sym->sym_feature == function_symbol || sym->sym_feature == procedure_symbol) {
      for(p = sym->sym_params; p != NULL; p = p->param_next) {
        fprintf(stdout, "%u%s ", p->param_count, (p->param_feature == val_parameter_symbol) ? "val" : "ref");
      }
    }

    /* Imprime o nome do símbolo */
    fprintf(stdout, "- %s\n", sym->sym_name);
  }

  fprintf(stdout, "\n");
}

/* Libera a memória ocupada pelos símbolos do nível léxico atual e retorna a quantidade de símbolos removidos */
void free_level_symbols(unsigned int level) {
  struct symbol_table *sym, *free_sym, *prev_sym;
  unsigned int dmem_count = 0;

  prev_sym = NULL;
  sym = sym_tb_base;

  /* Percorre todos os símbolos da tabela */
  while(sym != NULL) {
    free_sym = NULL;

    /* Verifica a caracteristica do símbolo */
    switch(sym->sym_feature) {
      /* Se é variável, remove se o nível léxico é maior ou igual ao atual e incrementa a contagem
         de memória para ser liberada posteriormente */
      case variable_symbol:
        if(sym->sym_lex_level >= level) {
          free_sym = sym;
          ++dmem_count;
        }

        break;

      /* Se for um parâmetro, remove se é do nível léxico maior ou igual ao atual */
      case val_parameter_symbol:
      case ref_parameter_symbol:
        if(sym->sym_lex_level >= level) {
          free_sym = sym;
        }

        break;

      /* Se for função ou procedimento, remove apenas se o nível léxico é maior que o atual */
      case procedure_symbol:
      case function_symbol:
        if(sym->sym_lex_level > level) {
          free_sym = sym;
        }
    }

    /* Se o símbolo não for removido, define-o como anterior (ponteiro usado em caso de remoção) */
    if(free_sym == NULL) {
      prev_sym = sym;
    }

    /* Aponta para o próximo símbolo (próxima iteração) */
    sym = sym->sym_next;

    /* Se o símbolo deve ser removido, então ajeita a tabela e libera a memória ocupada pelo mesmo */
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

  /* Se há memória de variável para ser liberada, gera o código para isso */
  if(dmem_count > 0) {
    generate_code(NULL, "DMEM %u", dmem_count);
  }
}

/* Libera a memória ocupada pela tabela de símbolos */
void free_symbols() {
  struct symbol_table *sym, *aux_sym;
  struct param_list *prev_params;

  sym = sym_tb_base;

  /* Percorre toda a tabela */
  while(sym != NULL) {
    aux_sym = sym;
    sym = sym->sym_next;

    /* Libera o espaço ocupado pelo nome se não for nulo */
    if(aux_sym->sym_name != NULL) {
      free(aux_sym->sym_name);
    }

    /* Libera os parâmetros do símbolo se não forem nulos */
    while(aux_sym->sym_params != NULL) {
      prev_params = aux_sym->sym_params;
      aux_sym->sym_params = aux_sym->sym_params->param_next;
      free(prev_params);
    }

    /* Libera o espaço ocupado pelo nodo na tabela */
    free(aux_sym);
  }

  sym_tb_base = NULL;
}

/* Empilha um valor genérico (nesse caso é usado void *) */
void push(struct stack_node **stack, void *value) {
  struct stack_node *next;

  /* Topo atual da pilha */
  next = *stack;

  /* Aloca o nodo do elemento a ser empilhado e define-o como topo da pilha */
  *stack = (struct stack_node *) malloc(sizeof(struct stack_node));

  /* Armazena o endereço do nodo e o próximo como o topo anterior da pilha */
  if(*stack != NULL) {
    (*stack)->stack_next = next;
    (*stack)->stack_value = value;
  }
}

/* Retira um elemento da pilha */
void *pop(struct stack_node **stack) {
  void *ret;
  struct stack_node *aux;

  /* Se a pilha está vazia, retorna nulo */
  if(*stack == NULL) {
    return NULL;
  }

  /* Armazena o valor do topo da pilha no retorno e seu nodo em uma variavel auxiliar */
  ret = (*stack)->stack_value;
  aux = *stack;

  /* Aponta o topo da pilha para o próximo elemento e libera o espaço ocupado pelo nodo */ 
  *stack = (*stack)->stack_next;
  free(aux);
  return ret;
}

/* Empilha um valor inteiro (signed) */
void ipush(struct stack_node **stack, int value) {
  int *iptr;

  /* Aloca um espaço de memória para armazenar o endereço do inteiro e empilha-o */
  iptr = (int *) malloc(sizeof(int));
  *iptr = value;
  push(stack, iptr);
}

/* Retira um valor inteiro (signed) da pilha */
int ipop(struct stack_node **stack) {
  int *iptr;
  int ret;

  /* Retira o valor de endereço do topo da pilha, fazendo um casting para int * e salva o resultado */
  iptr = pop(stack);
  ret = *iptr;

  /* Libera o espaço ocupado pelo endereço e retorna o resultado salvo */
  free(iptr);
  return ret;
}

/* Empilha um valor inteiro não assignalado (unsigned) */
void uipush(struct stack_node **stack, unsigned int value) {
  unsigned int *uiptr;

  /* Aloca um espaço de memória para armazenar o endereço do inteiro e empilha-o */
  uiptr = (unsigned int *) malloc(sizeof(unsigned int));
  *uiptr = value;
  push(stack, uiptr);
}

/* Retira um valor inteiro (unsigned) da pilha */
unsigned int uipop(struct stack_node **stack) {
  unsigned int *uiptr;
  unsigned int ret;

  /* Retira o valor de endereço do topo da pilha, fazendo um casting para unsigned int * e salva o resultado */
  uiptr = pop(stack);
  ret = *uiptr;

  /* Libera o espaço ocupado pelo endereço e retorna o resultado salvo */
  free(uiptr);
  return ret;
}

/* Processa o tipo de uma pilha de tipos (verifica se corresponde ao esperado e empilha no próximo estágio) */
void process_stack_type(struct stack_node **stack, symbol_type type, struct stack_node **dest) {
  symbol_type tstk;

  /* Tipo do topo da pilha */
  tstk = (symbol_type) ipop(stack);

  /* Se o tipo não for o esperado, exibe erro de compilação de finaliza o compilador */
  if(tstk != type) {
    print_error("Invalid expression type, expected '%s', found '%s'\n", get_symbol_type_string(type), get_symbol_type_string(tstk));
  }

  /* Se o destino (pilha do próximo estágio) não é nulo, empilha o tipo no mesmo */
  if(dest != NULL) {
    ipush(dest, (int) tstk);
  }
}

/* Transfere um tipo de dados de uma pilha à outra */
void transfer_stack_type(struct stack_node **source, struct stack_node **dest) {
  ipush(dest, ipop(source));
}

/* Insere parâmetros em uma lista de parâmetros */
void insert_params(struct param_list **dest, unsigned int nparams, symbol_type type, symbol_feature feature) {
  struct param_list *p, *last;
  unsigned int i;

  /* ALoca o bloco contendo as informações dos parâmetros */
  p = (struct param_list *) malloc(sizeof(struct param_list));

  /* Se o bloco foi alocado com sucesso */
  if(p != NULL) {
    /* Atribui os dados do bloco alocado */
    p->param_feature = feature;
    p->param_type = type;
    p->param_count = nparams;
    p->param_next = NULL;

    /* Insere o bloco no final da lista de parâmetros (se a base não existir define-o como a mesma) */
    if(*dest == NULL) {
      *dest = p;
    } else {
      for(last = *dest; last->param_next != NULL; last = last->param_next);
      last->param_next = p;
    }
  }
}

/* Obtêm a característica de parâmetro dada sua posição na lista */
symbol_feature get_param_feature(struct param_list *p, unsigned int param_no) {
  unsigned int i;

  /* Percorre a lista de parâmetros até encontrar o intervalo a qual pertence a posição */ 
  for(i = 0; i + p->param_count <= param_no; i += p->param_count) {
    if(p->param_next == NULL) {
      print_error("Unexpected parameter for function (%d).\n", param_no);
    }

    p = p->param_next;
  }

  /* Retorna a característica encontrada */
  return p->param_feature;
}

/* Verifica se o tipo do parâmetro dada sua posição na lista é igual ao tipo especificado */
void check_param(struct param_list *p, unsigned int param_no, symbol_type type) {
  unsigned int i;

  /* Percorre a lista de parâmetros até encontrar o intervalo a qual pertence a posição */ 
  for(i = 0; i + p->param_count <= param_no; i += p->param_count) {
    if(p->param_next == NULL) {
      print_error("Unexpected parameter for function (%d).\n", param_no);
    }

    p = p->param_next;
  }

  /* Verifica se o tipo é igual ao tipo especificado, se não for, retorna uma mensagem de erro e finaliza o compilador */
  if(p->param_type != type) {
    print_error("Invalid parameter %d type, expected '%s', found '%s'.\n", param_no, get_symbol_type_string(p->param_type), get_symbol_type_string(type));
  }
}

/* Obtêm o valor do próximo rótulo */
unsigned int get_next_label() {
  static unsigned int current_label = 0;
  return current_label++;
}

/* Gera o código para definir o rótulo especificado */
void generate_label(unsigned int label) {
  char label_str[MAX_LABEL];
  snprintf(label_str, sizeof label_str, "R%s%u", (label < 10) ? "0" : "", label);
  generate_code(label_str, "NADA ");
}

/* Retorna a string do rótulo especificado */
char *get_label_string(unsigned int label) {
  static char label_str[MAX_LABEL];
  snprintf(label_str, sizeof label_str, "R%s%u", (label < 10) ? "0" : "", label);
  return label_str;
}
