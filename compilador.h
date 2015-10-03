/* Tamanho máximo de um símbolo no código Pascal */
#define MAX_TOKEN         16

/* Tamanho máximo de uma referência à algum símbolo no código MEPA */
#define MAX_SYMBOL_REF    32

typedef enum {
  SYM_NULL = 0,
  SYM_VAR, 
  SYM_FUNCTION 
} symbol_type;

typedef enum { 
  sym_program, sym_var, sym_begin, sym_end, 
  sym_identifier, sym_number,
  sym_dot, sym_comma, sym_semicolon, sym_colon,
  sym_set, sym_parentheses_open, sym_parentheses_close,
} symbol_name;

struct symbol_table {
  char *sym_name;
  symbol_type sym_type;
  int sym_lex_level;
  int sym_offset;
  struct symbol_table *next;
};

extern symbol_name simbolo, relacao;
extern unsigned int lexical_level;
extern unsigned int var_offset;
extern unsigned int line_number;

symbol_name symbol, relation;
char token[MAX_TOKEN];

void generate_code(const char *label, const char *code);
void print_error(const char *error);

void insert_symbol(const char *name, symbol_type type);
const char *get_symbol_ref(const char *name);
