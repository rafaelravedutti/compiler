/* Tamanho máximo de um símbolo no código Pascal */
#define MAX_TOKEN         32

/* Tamanho máximo de uma referência à algum símbolo no código MEPA */
#define MAX_SYMBOL_REF    32

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
  sym_div, sym_mod
} symbol_name;

typedef enum {
  null_symbol = 0,
  variable_symbol,
  function_symbol 
} symbol_feature;

typedef enum {
  sym_type_null = 0,
  sym_type_integer,
  sym_type_boolean
} symbol_type;

struct symbol_table {
  char *sym_name;
  symbol_feature sym_feature;
  symbol_type sym_type;
  int sym_lex_level;
  int sym_offset;
  struct symbol_table *sym_next;
};

struct stack_node {
  void *stack_value;
  struct stack_node *stack_next;
};

/* Variáveis globais */
char token[MAX_TOKEN];
char variable_name[MAX_TOKEN];
char *variable_reference;
char *function_reference;
struct stack_node *expr_stack;
struct stack_node *term_stack;
struct stack_node *factor_stack;
symbol_name symbol;
symbol_name relation;
symbol_type symbol_type_id;
unsigned int block_variables;
unsigned int line_variables;

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
struct symbol_table *create_symbol(const char *name, symbol_feature feature);
struct symbol_table *find_symbol(const char *name);
char *get_symbol_reference(const char *name);
void set_last_symbols_type(unsigned int nsymbols, symbol_type tsym);
void print_symbols_table();
unsigned int free_level_symbols();
void free_symbols();

/* Funções de pilha */
void push(struct stack_node **stack, void *value);
void *pop(struct stack_node **stack);
void ipush(struct stack_node **stack, int value);
int ipop(struct stack_node **stack);

/* Funções de pilha de tipos de dados */
void process_stack_type(struct stack_node **stack, symbol_type type, struct stack_node **dest);
void transfer_stack_type(struct stack_node **source, struct stack_node **dest);
