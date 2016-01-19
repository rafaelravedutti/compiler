/* Tamanho máximo de um símbolo no código Pascal */
#define MAX_TOKEN         32

/* Tamanho máximo de um rótulo */
#define MAX_LABEL         8

/* Tamanho máximo de uma referência à algum símbolo no código MEPA */
#define MAX_SYMBOL_REF    32

/* Nomes de símbolos */
typedef enum { 
  sym_program, sym_var, sym_begin, sym_end, sym_while,
  sym_do, sym_for, sym_to, sym_downto, sym_if, sym_then,
  sym_else, sym_procedure, sym_function, sym_repeat,
  sym_until, sym_goto, sym_label, sym_not, sym_case,
  sym_in, sym_identifier, sym_number,
  sym_dot, sym_comma, sym_semicolon, sym_colon,
  sym_set, sym_parentheses_open, sym_parentheses_close,
  sym_equal, sym_diff, sym_less_than, sym_higher_than,
  sym_less_or_equal_than, sym_higher_or_equal_than,
  sym_and, sym_or, sym_sum, sym_sub, sym_times,
  sym_div, sym_mod, sym_true, sym_false,
  sym_write, sym_read, sym_div_int
} symbol_name;

/* Caracteristicas de símbolos */
typedef enum {
  null_symbol = 0,
  variable_symbol,
  function_symbol,
  procedure_symbol,
  label_symbol,
  val_parameter_symbol,
  ref_parameter_symbol 
} symbol_feature;

/* Tipos de símbolos */
typedef enum {
  sym_type_null = 0,
  sym_type_integer,
  sym_type_boolean
} symbol_type;

/* Estrutura da tabela de símbolos */
struct symbol_table {
  char *sym_name;
  symbol_feature sym_feature;
  symbol_type sym_type;
  int sym_offset;
  unsigned int sym_lex_level;
  unsigned int sym_label;         /* functions/procedures only */
  unsigned int sym_nparams;       /* functions/procedures only */
  struct param_list *sym_params;  /* functions/procedures only */
  struct symbol_table *sym_next;
};

/* Lista de parâmetros */
struct param_list {
  symbol_feature param_feature;
  symbol_type param_type;
  unsigned int param_count;
  struct param_list *param_next;
};

/* Estrutura de pilha genérica */
struct stack_node {
  void *stack_value;
  struct stack_node *stack_next;
};

/* Variáveis globais */
char token[MAX_TOKEN];
struct symbol_table *variable_ptr, *subroutine_ptr;
struct stack_node *expr_stack, *term_stack, *factor_stack;
struct stack_node *if_stack, *while_stack;
struct stack_node *variable_stack, *subroutine_stack;
struct stack_node *relation_stack;
unsigned int block_variables, line_variables;
unsigned int block_parameters, line_parameters;
unsigned int if_label, if_not_label;
unsigned int while_inner_label, while_outter_label;
unsigned int subroutine_label;
symbol_name symbol, relation;
symbol_type symbol_type_id;
symbol_feature param_feature, subroutine_feature;

/* Variáveis externas */
extern unsigned int lexical_level;
extern unsigned int line_number;

/* Funções do bison */
int yylex(void);
void yyerror(const char *);

/* Funções de entrada e saída do compilador */
void generate_code(const char *label, const char *code, ...);
void print_error(const char *error, ...);

/* Funções de tipos de dados */
symbol_type parse_type(const char *type);

/* Funções da tabela de símbolos */
struct symbol_table *create_symbol(const char *name, symbol_feature feature, unsigned int label);
struct symbol_table *find_symbol(const char *name, symbol_feature feature, int must_exist);
struct symbol_table *find_variable_or_parameter(const char *name);
void set_last_symbols_type(unsigned int nsymbols, symbol_type tsym);
void set_parameters_offset(unsigned int nsymbols);
void print_symbols_table();
void free_level_symbols();
void free_symbols();

/* Funções de pilha */
void push(struct stack_node **stack, void *value);
void *pop(struct stack_node **stack);
void ipush(struct stack_node **stack, int value);
int ipop(struct stack_node **stack);
void uipush(struct stack_node **stack, unsigned int value);
unsigned int uipop(struct stack_node **stack);

/* Funções de pilha de tipos de dados */
void process_stack_type(struct stack_node **stack, symbol_type type, struct stack_node **dest);
void transfer_stack_type(struct stack_node **source, struct stack_node **dest);

/* Funções da lista de parâmetros */
void insert_params(struct param_list **dest, unsigned int nparams, symbol_type type, symbol_feature feature);
symbol_feature get_param_feature(struct param_list *p, unsigned int param_no);
void check_param(struct param_list *p, unsigned int param_no, symbol_type type);

/* Funções de rótulos */
unsigned int get_next_label();
void generate_label(unsigned int label);
char *get_label_string(unsigned int label);
