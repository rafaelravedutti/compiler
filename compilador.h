/* Tamanho máximo de um símbolo no código Pascal */
#define MAX_TOKEN         32

/* Tamanho máximo de uma referência à algum símbolo no código MEPA */
#define MAX_SYMBOL_REF    32

typedef enum { 
  sym_program, sym_var, sym_begin, sym_end, sym_while,
  sym_for, sym_to, sym_downto, sym_if, sym_then,
  sym_else, sym_procedure, sym_function, sym_repeat,
  sym_until, sym_goto, sym_label, sym_not, sym_case,
  sym_in, sym_identifier, sym_number,
  sym_dot, sym_comma, sym_semicolon, sym_colon,
  sym_set, sym_parentheses_open, sym_parentheses_close,
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

/* Variáveis globais */
char token[MAX_TOKEN];
symbol_name symbol, relation;

/* Variáveis externas */
extern unsigned int lexical_level;
extern unsigned int var_offset;
extern unsigned int line_number;
extern unsigned int declared_variables;

/* Funções de entrada e saída do compilador */
void generate_code(const char *label, const char *code, ...);
void print_error(const char *error, ...);

/* Funções da tabela de símbolos */
void create_symbol(const char *name, symbol_type type);
const char *get_symbol_ref(const char *name);
void free_level_symbols();
void free_symbols();
