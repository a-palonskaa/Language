#ifndef BACKEND_H
#define BACKEND_H

#include "prog_tree.h"

class backend_t {
public:
    void init(FILE* istream);
    void dtor();
    void translate_to_asm(FILE* ostream);

    void set_dump_ostream(FILE* ostream);
    void dump();

    void print_func(FILE* ostream, node_t* decl);
    void print_func_body(FILE* ostream, node_t* node, size_t ram_index);
    void print_expr(FILE* ostream, node_t* node, size_t ram_index);
    void push_expr(FILE* ostream, node_t* node, size_t ram_index);
    void print_op_expr_r(FILE* ostream, node_t* node, size_t ram_index);
    void print_op(FILE* ostream, node_t* node, size_t ram_index);
    void print_if_else(FILE* ostream, node_t* node, size_t ram_index);
    void print_equal(FILE* ostream, node_t* node, size_t ram_index);
    const char* reg_by_num(size_t i);
private:
    prog_tree_t prog_tree_;
};

#endif /* BACKEND_H */
