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
  sym_type_null = 0,
  sym_type_var, 
  sym_type_function 
} symbol_type;

struct symbol_table {
  char *sym_name;
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
char *ident_ref;
symbol_name symbol, relation;
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

/* Funções da tabela de símbolos */
void create_symbol(const char *name, symbol_type type);
char *get_symbol_ref(const char *name);
unsigned int free_level_symbols();
void free_symbols();

/* Funções de pilha */
void push(struct stack_node **stack, void *value);
void *pop(struct stack_node **stack);
void ipush(struct stack_node **stack, int value);
int ipop(struct stack_node **stack);
