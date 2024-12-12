#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "text_lib.h"

#define MAX_OP_LEN 20
#define MAX_NAME_LEN 21

typedef enum {
    NUM = 0,
    VAR = 1,
    OP  = 2,
} type_t;

typedef enum {
    POISON = -1,
    ADD    = 0,
    SUB    = 1,
    MUL    = 2,
    DIV    = 3,
    POW    = 4,
    LOG    = 5,
    LN     = 6,
    EXP    = 7,
    SIN    = 8,
    COS    = 9,
    TG     = 10,
    CTG    = 11,
    SH     = 12,
    CH     = 13,
    TH     = 14,
    CTH    = 15,
    ARCSIN = 16,
    ARCCOS = 17,
    ARCTG  = 18,
    ARCCTG = 19,
    ARCSH  = 20,
    ARCCH  = 21,
    ARCTH  = 22,
    ARCCTH = 23,

    EQ   = 50, // =
    IE   = 51, // ==
    INE  = 52, // !=
    IA   = 53, // >
    IAEQ = 54, // >=
    IB   = 55, // <
    IBEQ = 56, // <=

    IF     = 75,
    ELSE   = 76,
    WHILE  = 77,
    RETURN = 78,

    BRACKET_OPEN     = 100,
    BRACKET_CLOSE    = 101,
    CODE_BLOCK_OPEN  = 102,
    CODE_BLOCK_CLOSE = 103,
    SEMICOLON        = 104,

    EOT           = 1000,
} op_t;

typedef struct {
    char name[MAX_NAME_LEN];
    bool initialized;
} name_t;

struct node_t {
    node_t* parent;
    node_t* left;
    node_t* right;

    type_t type;
    double value;
};

typedef enum {
    NO_ERR             = 0,
    SYNTAX_ERR         = 1,
    MEM_ALLOC_ERR      = 2,
    NUM_INVAR_ERR      = 3,
    UN_OP_INVAR_ERR    = 4,
    BIN_OP_INVAR_ERR   = 5,
    SUB_SYNTAX_ERR     = 6,
    ADD_SYNTAX_ERR     = 7,
    INVALID_ROOT_ERR   = 8,
    CYCLIC_LINKING_ERR = 9,
} err_t;

typedef enum {
    RIGHT = 0,
    LEFT  = 1,
    ROOT  = 2,
} rel_t;

typedef struct {
    char name[MAX_NAME_LEN];
    op_t code;
} func_name_table_t;

const func_name_table_t func_name_table[] = {
    { "+",      ADD},
    { "-",      SUB},
    { "/",      DIV},
    { "*",      MUL},
    { "^",      POW},
    { "log",    LOG},
    { "ln",     LN},
    { "exp",    EXP},
    { "arcsin", ARCSIN},
    { "arccos", ARCCOS},
    { "arctg",  ARCTG},
    { "arcctg", ARCCTG},
    { "arcch",  ARCCH},
    { "arcsh",  ARCSH},
    { "arcth",  ARCTH},
    { "arccth", ARCCTH},
    { "sin",    SIN},
    { "cos",    COS},
    { "tg",     TG},
    { "ctg",    CTG},
    { "sh",     SH},
    { "ch",     CH},
    { "cth",    CTH},
    { "th",     TH},
    { "=",      EQ},
    { "==",     IE},
    { "!=",     INE},
    { ">",      IA},
    { ">=",     IAEQ},
    { "<",      IB},
    { "<=",     IBEQ},
    { "if",     IF},
    { "else",   ELSE},
    { "while",  WHILE},
    { "return", RETURN},
    { "(",      BRACKET_OPEN},
    { ")",      BRACKET_CLOSE},
    { "{",      CODE_BLOCK_OPEN},
    { "}",      CODE_BLOCK_CLOSE},
    { ";",      SEMICOLON},
    { "$",      EOT}};

const size_t func_name_table_len = sizeof(func_name_table) / sizeof(func_name_table[0]);

typedef struct {
    char name[MAX_NAME_LEN];
    node_t* tree;
    name_t var_nametable_[10];
    size_t var_nametable_size_;
} new_func_name_t;

class prog_tree_t {
public:
    err_t init(FILE* data_file);
    void dtor();

    void set_dump_ostream(FILE* ostream);
    void print_preorder_();
    void print_inorder_();
    void print_preorder(node_t* node);

    void print_links(FILE* tree_file, node_t* node);
    void print_nodes(FILE* tree_file, node_t* node, size_t rank);
    void printf_tree_dot_file(FILE* tree_file, node_t* node);
    void dump(node_t* root);
    void dump_tree();

    int def_operator(char* op);

    void print_tree_to_tex(FILE* ostream, node_t* root);
    void print_exp_to_tex(FILE* ostream, node_t* node);
private:
    int get_operator_precedence(int op);
    void print_to_tex(FILE* ostream, node_t* node);
    void print_operator(FILE* ostream, double value);
    void print_inorder(FILE* ostream, node_t* node, int parent_precedence);

// Grammar
    void syntax_error(size_t p, const char* func, size_t line);

    node_t* get_gram();
    node_t* get_asgn();
    node_t* get_if();
    node_t* get_else();
    node_t* get_op();
    node_t* get_expr();
    node_t* get_term();
    node_t* get_basic_expr();
    node_t* get_num();
    node_t* get_id();
    node_t* get_func();

    node_t* token_init(text_t* text);
    node_t* link_tokens();
    void tokenize_text(text_t* text);
    err_t parse_identificator(text_t* text, size_t* ip, node_t* node);
    op_t is_operator(char* name);
    bool is_unary(double value);
    void parse_number(text_t* text, size_t* ip, node_t* node);
    void initialize_op_node(node_t* node, double val);
    void print_tokens_array();

    double find_name_in_nametable(char* name);
    double add_name_to_nametable(char* name);
    double index_in_nametable(char* name);
    void print_var_nametable();
    bool parse_operator(text_t* text, size_t* ip, node_t* node);
    const char* op_name(int val);
    bool is_compare(int val);
private:
    node_t* root_{nullptr};

    name_t var_nametable_[100];
    size_t var_nametable_size_{0};

    new_func_name_t func_nametable_[10];
    size_t func_nametable_size_{0};

    name_t* current_var_nametable_{var_nametable_};

    size_t ip_{0};
    node_t* tokens_{nullptr};
    size_t tokens_array_size_{0};
};

#endif /* EXPRESSION_TREE_H */
