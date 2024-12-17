#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

node_t* prog_tree_t::get_new_var() {
    size_t old_ip = ip_;
    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != DEF_VAR) {
        return nullptr;
    }
    node_t* var = &tokens_[ip_];
    ip_++;

    var_nametable_[(int) tokens_[ip_].value].initialized = true; //NOTE -  check if there is a var

    node_t* asgn = get_asgn();
    if (asgn == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    var->right = asgn;
    asgn->parent = var;
    return var;
}

node_t* prog_tree_t::get_gram() {
    node_t* root = nullptr;

    node_t* new_func = get_new_func_();
    node_t* semicolon = nullptr;
    node_t* semicolon_parent = nullptr;

    while (new_func != nullptr) {
        if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != SEMICOLON) {
            _syntax_error();
            return root;
        }
        semicolon = &tokens_[ip_];
        ip_++;

        semicolon->left = new_func;
        new_func->parent = semicolon;

        if (semicolon_parent == nullptr) {
            root = semicolon;
        }
        else {
            semicolon_parent->right = semicolon;
            semicolon->parent = semicolon_parent;
        }

        semicolon_parent = semicolon;
        new_func = get_new_func_();
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != EOT) {
        _syntax_error();
        return root;
    }
    ip_++;
    return root;
}

ssize_t prog_tree_t::add_func_to_nametable(node_t* func) {
    if (func_nametable_size_ == 10) {  // NOTE - define 10;
        LOG(ERROR, "Array of new funcs is full, cannot add more\n");
        return -1;
    }

    strcpy(func_nametable_[func_nametable_size_].name, var_nametable_[(int) func->value].name);
    func_nametable_[func_nametable_size_].var_nametable_index = (size_t) func->value;
    func_nametable_size_++;
    return (ssize_t) func_nametable_size_ - 1;
}

node_t* prog_tree_t::get_new_func_() {
    size_t old_ip = ip_;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != DECL) {
        return nullptr;
    }
    node_t* decl = &tokens_[ip_];
    ip_++;

    node_t* func = get_id();
    func->type = FUNC;

    if (func == nullptr) {
        ip_ = old_ip;
        _syntax_error();
        return nullptr;
    }

    ssize_t new_func_id = add_func_to_nametable(func);
    if (new_func_id == -1) {
        _syntax_error();
        return nullptr;
    }

    size_t func_id = (size_t) new_func_id;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_OPEN) {
        ip_ = old_ip;
        return nullptr;
    }
    node_t* first_bracket = &tokens_[ip_];
    ip_++;

    first_bracket->type = OP;
    first_bracket->value = SPEC;

    decl->left = first_bracket;
    first_bracket->parent = decl;
    first_bracket->left = func;
    func->parent = first_bracket;


    //TODO -  first_bracket->right = ... variable
    if (tokens_[ip_].type == VAR) {
        if (tokens_[ip_ + 1].type == OP && (int) tokens_[ip_ + 1].value == BRACKET_CLOSE) {
            ip_++;
            first_bracket->right = &tokens_[ip_];
            tokens_[ip_].parent = first_bracket;
            tokens_[ip_].value = SEMICOLON;

            tokens_[ip_].left = &tokens_[ip_ - 1];
            tokens_[ip_ - 1].parent = &tokens_[ip_];
            ip_++;
        }
        else {
            node_t* var_node = &tokens_[ip_++];
            node_t* semicolon = &tokens_[ip_];
            node_t* new_semicolon = semicolon;
            node_t* past_semicolon = nullptr;

            first_bracket->right = semicolon;
            semicolon->parent = first_bracket;

            while (tokens_[ip_].type == OP && (int) tokens_[ip_].value == SEMICOLON) {
                new_semicolon = &tokens_[ip_];

                if (past_semicolon != nullptr) {
                    past_semicolon->right = new_semicolon;
                    new_semicolon->parent = past_semicolon;
                }

                new_semicolon->left = var_node;
                var_node->parent = new_semicolon;
                ip_++;

                var_node = &tokens_[ip_];
                ip_++;
                past_semicolon = new_semicolon;
            }

            new_semicolon->right = var_node; // FIXME - check if var

            if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_CLOSE) {
                ip_ = old_ip;
            return nullptr;
            }
            ip_++;
        }
    }
    else {
        if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_CLOSE) {
            ip_ = old_ip;
            return nullptr;
        }
        ip_++;
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != CODE_BLOCK_OPEN) {
        ip_ = old_ip;
        return nullptr;
    }
    ip_++;

    node_t* root = nullptr;
    node_t* parent_node = nullptr;

    node_t* val = get_op();
    if (val == nullptr) {
        _syntax_error();
        return nullptr;
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != SEMICOLON) {
        _syntax_error();
        return nullptr;
    }

    root = &tokens_[ip_];
    decl->right = root;
    root->parent = decl;

    ip_++;

    root->left = val;
    val->parent = root;
    parent_node = root;

    while (1) {
        val = get_op();
        if (val == nullptr) break;

        if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != SEMICOLON) {
            return nullptr;
        }

        parent_node->right = &tokens_[ip_];
        tokens_[ip_].parent = parent_node;

        tokens_[ip_].left = val;
        val->parent = &tokens_[ip_];

        parent_node = &tokens_[ip_];
        ip_++;
    }

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != CODE_BLOCK_CLOSE) {
        return nullptr;
    }
    ip_++;

    func_nametable_[func_id].tree = root;

    decl->right = root;
    root->parent = decl;
    return decl;
}

node_t* prog_tree_t::get_new_func() {
    size_t old_ip = ip_;

    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != CALL) {
        return nullptr;
    }
    node_t* call = &tokens_[ip_];
    ip_++;

    node_t bracket = {};
    node_t* first_bracket = &bracket;
    node_t* func = nullptr;
    node_t* semicolon = nullptr;
    for (size_t i = 0; i < func_nametable_size_; i++) {
        if ((size_t) tokens_[ip_].value == func_nametable_[i].var_nametable_index) {
            func = &tokens_[ip_];
            ip_++;
            if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_OPEN) {
                ip_ = old_ip;
                return nullptr;
            }
            ip_++;

            node_t* var_node = nullptr;
            if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_CLOSE) {
                var_node = get_expr();
                if (var_node == nullptr) {
                    var_node = get_id();
                    if (var_node == nullptr) {
                        return nullptr;
                    }
                }

                semicolon = &tokens_[ip_];
                node_t* new_semicolon = semicolon;
                node_t* past_semicolon = nullptr;

                first_bracket->right = semicolon;
                semicolon->parent = first_bracket;

                while (tokens_[ip_].type == OP && (int) tokens_[ip_].value == SEMICOLON) {
                    new_semicolon = &tokens_[ip_];

                    if (past_semicolon != nullptr) {
                        past_semicolon->right = new_semicolon;
                        new_semicolon->parent = past_semicolon;
                    }

                    new_semicolon->left = var_node;
                    var_node->parent = new_semicolon;
                    ip_++;

                    var_node = get_expr();
                    if (var_node == nullptr) {
                        var_node = get_id();
                        if (var_node == nullptr) {
                            return nullptr;
                        }
                    }
                    past_semicolon = new_semicolon;
                }

                new_semicolon->right = var_node;
            }

            if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != BRACKET_CLOSE) {
                ip_ = old_ip;
                return nullptr;
            }

            tokens_[ip_].value = SEMICOLON;
            tokens_[ip_].right = semicolon->right;
            tokens_[ip_].left = semicolon->left;
            if (semicolon->left != nullptr) semicolon->left->parent = &tokens_[ip_];
            if (semicolon->right != nullptr) semicolon->right->parent = &tokens_[ip_];
            call->left = &tokens_[ip_];
            tokens_[ip_].parent = call;
            ip_++;

            call->right = func;
            func->parent = call;
            return call; //NOTE - maybe copy myrr meow
        }
    }
    return nullptr;
}

node_t* prog_tree_t::get_asgn() {
    size_t old_ip = ip_;

    node_t* val = get_id();
    if (val == nullptr) {
        ip_ = old_ip;
        return nullptr;
    }

    if (var_nametable_[(int) val->value].initialized == false) {
        LOG(ERROR, "Uninitialized variable\n");
        _syntax_error();
        ip_ = old_ip;
        return nullptr;
    }

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

node_t* prog_tree_t::get_ret() {
    if (tokens_[ip_].type != OP || (int) tokens_[ip_].value != RETURN) {
        return nullptr;
    }
    return &tokens_[ip_++];
}

node_t* prog_tree_t::get_op() {
    node_t* val = get_if();
    if (val != nullptr) {
        return val;
    }

    val = get_ret();
    if (val != nullptr) {
        return val;
    }

    val = get_new_var();
    if (val != nullptr) {
        return val;
    }

    val = get_new_func();
    if (val != nullptr) {
        return val;
    }

    val = get_in_out();
    if (val != nullptr) {
        return val;
    }

    return get_asgn(); //NOTE - maybe nullptr
}

node_t* prog_tree_t::get_in_out() {
    if (!(tokens_[ip_].type == OP && ((int) tokens_[ip_].value == IN || (int) tokens_[ip_].value == OUT))) {
        return nullptr;
    }
    node_t* cmd_node = &tokens_[ip_];

    ip_++;

    if (tokens_[ip_].type != VAR) {
        _syntax_error();
        return nullptr;
    }

    cmd_node->left = &tokens_[ip_];
    tokens_[ip_].parent = cmd_node;
    ip_++;
    return cmd_node;
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

