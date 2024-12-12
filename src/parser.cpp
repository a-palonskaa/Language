#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog_tree.h"
#include "logger.h"

#define _syntax_error() syntax_error(ip_, __func__, __LINE__)

void prog_tree_t::syntax_error(size_t p, const char* func, size_t line) {
    LOG(ERROR, "Syntax error p = %zu, type = %d(val = %f) func: %s (%zu)\n""%s\n",
               p, tokens_[p].type, tokens_[p].value, func, line, op_name((int)tokens_[p].value));
    exit(0);
}

const char* prog_tree_t::op_name(int val) {
    for (size_t i = 0; i < func_name_table_len; i++) {
        if (func_name_table[i].code == val) {
            return func_name_table[i].name;
        }
    }
    return nullptr;
}

node_t* prog_tree_t::get_gram() {
    ip_ = 0;
    node_t* root = nullptr;
    node_t* parent_node = nullptr;

    node_t* val = get_op();
    if (val == nullptr) {
        return root;
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != SEMICOLON) {
        _syntax_error();
        return val;
    }

    root = &tokens_[ip_];
    ip_++;

    root->parent = nullptr;
    root->left = val;
    val->parent = root;
    parent_node = root;

    while (1) {
        val = get_op();
        if (val == nullptr) break;

        if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != SEMICOLON) {
            _syntax_error();
            return val;
        }

        parent_node->right = &tokens_[ip_];
        tokens_[ip_].parent = parent_node;

        tokens_[ip_].left = val;
        val->parent = &tokens_[ip_];

        parent_node = &tokens_[ip_];
        ip_++;
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != EOT) {
        _syntax_error();
        return root;
    }
    return root;
}

node_t* prog_tree_t::get_asgn() {
    size_t old_ip = ip_;

    node_t* val = get_id();
    if (val == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }
    var_nametable_[(int) val->value].initialized = true;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != EQ) {
        ip_ = old_ip;
        return nullptr;
    }
    node_t* root = &tokens_[ip_];

    ip_++;

    node_t* val1 = get_expr();
    if (val1 == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    root->right = val1;
    val1->parent = root;

    root->left = val;
    val->parent = root;

    return root; //NOTE - change parent of the root
}

bool prog_tree_t::is_compare(int val) {
    return val == IE || val == INE || val == IA || val == IAEQ || val == IB || val == IBEQ;
}

// IF::= 'if' '(' EXPR [IE INE IA IAEQ IB IBEQ] EXPR ')' ASGN {';' ELSE}

node_t* prog_tree_t::get_if() {
    size_t old_ip = ip_;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != IF) {
        ip_ = old_ip;
        return nullptr;
    }
    node_t* root = &tokens_[ip_];
    ip_++;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_OPEN) {
        ip_ = old_ip;
        return nullptr;
    }
    ip_++;

    node_t* val = get_expr();
    if (val == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    if (tokens_[ip_].type != OP || !is_compare((int) tokens_[ip_].value)) {
        ip_ = old_ip;
        return nullptr;
    }
    node_t* comp = &tokens_[ip_];
    ip_++;

    node_t* _val = get_expr();
    if (_val == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    comp->right = _val;
    comp->left  = val;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_CLOSE) {
        ip_ = old_ip;
        return nullptr;
    }
    ip_++;

    node_t* val1 = get_asgn();
    if (val1 == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    root->right = val1;
    val1->parent = root;

    root->left = comp;
    comp->parent = root;

    size_t old_ip2 = ip_;
    node_t* else_node = nullptr;
    if (tokens_[ip_].type == OP && (int) tokens_[ip_].value == SEMICOLON) {
        node_t* semicolon_node = &tokens_[ip_];
        ip_++;

        else_node = get_else();
        if (else_node == nullptr) {
            ip_ = old_ip2;
        }
        else {
            semicolon_node->right = else_node;
            else_node->parent = semicolon_node;

            semicolon_node->left = root;
            root->parent = semicolon_node;

            root = semicolon_node;
        }
    }

    return root; //NOTE -  change the parent of the root;
}

node_t* prog_tree_t::get_else() {
    size_t old_ip = ip_;
    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != ELSE) {
        return nullptr;
    }
    node_t* root = &tokens_[ip_];
    ip_++;

    node_t* val = get_asgn();
    if (val == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    root->right = val;
    val->parent = root;
    return root;
}

node_t* prog_tree_t::get_op() {
    node_t* val = get_if();
    if (val != nullptr) {
        return val;
    }

    return get_asgn(); //NOTE - maybe nullptr
}

node_t* prog_tree_t::get_expr() {
    node_t* val = get_term();
    while (tokens_[ip_].type == OP &&
          ((int) tokens_[ip_].value == ADD ||
           (int) tokens_[ip_].value == SUB)) {
        size_t op_ip = ip_;
        ip_++;
        node_t* val2 = get_term();

        tokens_[op_ip].left = val;
        val->parent = &tokens_[op_ip];

        tokens_[op_ip].right = val2;
        val2->parent = &tokens_[op_ip];

        val = &tokens_[op_ip];
    }
    return val;
}

node_t* prog_tree_t::get_term() {
    node_t* val = get_basic_expr();
    while (tokens_[ip_].type == OP &&
          ((int) tokens_[ip_].value == MUL ||
           (int) tokens_[ip_].value == DIV)) {
        size_t op_ip = ip_;
        ip_++;
        node_t* val2 = get_basic_expr();

        tokens_[op_ip].left = val;
        val->parent = &tokens_[op_ip];

        tokens_[op_ip].right = val2;
        val2->parent = &tokens_[op_ip];

        val = &tokens_[op_ip];
    }
    return val;
}

node_t* prog_tree_t::get_basic_expr() {
    node_t* val1 = nullptr;

    if (tokens_[ip_].type == OP && (int) tokens_[ip_].value == SUB) {
        size_t old_p = ip_;
        ip_++;
        tokens_[old_p].right = get_basic_expr();
        tokens_[old_p].left = nullptr;
        val1 = &tokens_[old_p];
    }
    else if (tokens_[ip_].type == OP && (int) tokens_[ip_].value == ADD) {
        size_t old_p = ip_;
        ip_++;
        tokens_[old_p].right = get_basic_expr();
        tokens_[old_p].left = nullptr;
        val1 = &tokens_[old_p];
    }
    else if (tokens_[ip_].type == OP && (int) tokens_[ip_].value == BRACKET_OPEN) {
        ip_++;
        node_t* val = get_expr();

        if (!(tokens_[ip_].type == OP && (int) tokens_[ip_].value == BRACKET_CLOSE)) {
            return nullptr;
        }

        ip_++;
        val1 = val;
    }
    else if ((val1 = get_num())  != nullptr ||
             (val1 = get_func()) != nullptr) {
        ;
    }
    else if ((val1 = get_id()) != nullptr) {
        if (var_nametable_[(int) val1->value].initialized == false) {
            LOG(ERROR, "Uninitialized variable \n");
            _syntax_error();
            return nullptr;
        }
    }
    else {
        return nullptr;
    }

    if (tokens_[ip_].type == OP && (int) tokens_[ip_].value == POW) {
        size_t old_p = ip_;
        ip_++;

        tokens_[old_p].left = val1;
        tokens_[old_p].right = get_basic_expr();

        val1 = &tokens_[old_p];
    }
    return val1;
}

node_t* prog_tree_t::get_num() {
    return (tokens_[ip_].type == NUM) ? &tokens_[ip_++] : nullptr;
}

node_t* prog_tree_t::get_id() {
    return (tokens_[ip_].type == VAR) ? &tokens_[ip_++] : nullptr;
}

node_t* prog_tree_t::get_func() {
    size_t old_p = ip_;

    if (tokens_[ip_].type == OP &&
       ((int) tokens_[ip_].value == SIN    ||
        (int) tokens_[ip_].value == COS    ||
        (int) tokens_[ip_].value == TG     ||
        (int) tokens_[ip_].value == CTG    ||
        (int) tokens_[ip_].value == SH     ||
        (int) tokens_[ip_].value == CH     ||
        (int) tokens_[ip_].value == TH     ||
        (int) tokens_[ip_].value == CTH    ||
        (int) tokens_[ip_].value == ARCSIN ||
        (int) tokens_[ip_].value == ARCCOS ||
        (int) tokens_[ip_].value == ARCTG  ||
        (int) tokens_[ip_].value == ARCCTG ||
        (int) tokens_[ip_].value == ARCSH  ||
        (int) tokens_[ip_].value == ARCCH  ||
        (int) tokens_[ip_].value == ARCTH  ||
        (int) tokens_[ip_].value == ARCCTH ||
        (int) tokens_[ip_].value == EXP    ||
        (int) tokens_[ip_].value == LN)) {
        ip_++;
        tokens_[old_p].left = get_basic_expr();
    }
    return (old_p == ip_) ? nullptr : &tokens_[old_p];
}

