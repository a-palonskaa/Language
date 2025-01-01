#ifndef MIDDLEEND_H
#define MIDDLEEND_H

#include "prog_tree.h"

class middleend_t {
public:
    void init(FILE* istream);
    void dtor();

    void set_dump_ostream(FILE* ostream);
    void dump();

    node_t* optimize(node_t* node);
    bool calculations_optimization_r(node_t* node);
    node_t* basic_operations_optimization_r(node_t* node, rel_t rel, bool* flag);
    node_t* null_val__optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag);
    node_t* one_val_optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag);

    void deserialization(FILE* ostream);
    double calculate_expression(node_t* node);
    void calculate_expression_r(node_t* node);
    double calculate_value(double op_type, node_t* node_l,  node_t* node_r);
private:
    prog_tree_t prog_tree_;
};

#endif /* MIDDLEEND_H */
