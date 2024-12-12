#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "text_lib.h"
#include "logger.h"
#include "prog_tree.h"

node_t* prog_tree_t::token_init(text_t* text) {
    assert(text != nullptr);

    tokens_ = (node_t*) calloc(sizeof(node_t), text->symbols_amount);
    if (tokens_ == nullptr) {
        LOG(ERROR, "Memory allocation error\n");
        return nullptr;
    }
    tokens_array_size_ = text->symbols_amount;

    tokenize_text(text);

    print_tokens_array();
    print_var_nametable();

    root_ = link_tokens();
    return root_;
}

node_t* prog_tree_t::link_tokens() {
    return get_gram();
}

void prog_tree_t::tokenize_text(text_t* text) {
    if (text == nullptr) {
        LOG(ERROR, "Text is nullptr\n");
        return;
    }

    size_t ip = 0;
    size_t i = 0;
    while (ip < text->symbols_amount) {
        while (isspace(text->symbols[ip])) {
            ip++;
        }

        if (text->symbols[ip] == '\0') break;

        if (isdigit(text->symbols[ip])) {
            parse_number(text, &ip, &tokens_[i]);
        }
        else if (parse_operator(text, &ip, &tokens_[i]) == true) {
            ;
        }
        else if (parse_identificator(text, &ip, &tokens_[i]) != NO_ERR) {
            printf("Syntax err %c(%d) %zu\n", text->symbols[ip], text->symbols[ip], text->symbols_amount);
            return;
        }
        i++;
    }
}

void prog_tree_t::print_tokens_array() {
    printf("\n\n\n\n\nDumping:");
    for (size_t i = 0; i < tokens_array_size_; i++) {
        printf("---\nnode[%p][%zu]:\n\ttype = %d\n\tval = %f\nr = %p,l = %p\n-----\n", &tokens_[i], i, tokens_[i].type, tokens_[i].value, tokens_[i].right, tokens_[i].left);
    }
}

bool prog_tree_t::parse_operator(text_t* text, size_t* ip, node_t* node) {
    assert(text != nullptr);
    assert(ip != nullptr);
    assert(node != nullptr);

    node->type = OP;
    node->parent = nullptr;
    node->right = nullptr;
    node->left = nullptr;

    switch (text->symbols[*ip]) {
        case '+': {
            node->value = ADD;
            break;
        }
        case '-': {
            node->value = SUB;
            break;
        }
        case '*': {
            node->value = MUL;
            break;
        }
        case '/': {
            node->value = DIV;
            break;
        }
        case '$': {
            node->value = EOT;
            break;
        }
        case ';': {
            node->value = SEMICOLON;
            break;
        }
        case '(': {
            node->value = BRACKET_OPEN;
            break;
        }
        case ')': {
            node->value = BRACKET_CLOSE;
            break;
        }
        case '{': {
            node->value = CODE_BLOCK_OPEN;
            break;
        }
        case '}': {
            node->value = CODE_BLOCK_CLOSE;
            break;
        }
        case '!': {
            if (text->symbols[*ip + 1] == '=') {
                (*ip)++;
                node->value = INE;
            }
            else {
                return false;
            }
            break;
        }
        case '=': {
            if (text->symbols[*ip + 1] == '=') {
                (*ip)++;
                node->value = IE;
            }
            else {
                node->value = EQ;
            }
            break;
        }
        case '>': {
            if (text->symbols[*ip + 1] == '=') {
                (*ip)++;
                node->value = IAEQ;
            }
            else {
                node->value = IA;
            }
            break;
        }
        case '<': {
            if (text->symbols[*ip + 1] == '=') {
                (*ip)++;
                node->value = IBEQ;
            }
            else {
                node->value = IB;
            }
            break;
        }
        default: {
            return false;
            break;
        }
    }
    (*ip)++;
    return true;
}

err_t prog_tree_t::parse_identificator(text_t* text, size_t* ip, node_t* node) {
    assert(text != nullptr);
    assert(ip != nullptr);
    assert(node != nullptr);

    size_t i = 0;
    char name[MAX_NAME_LEN] = "";
    while ((i < MAX_NAME_LEN) &&
           (isalpha(text->symbols[*ip]) ||
            isdigit(text->symbols[*ip]) ||
            text->symbols[*ip] == '_')) {
        name[i++] = (char) text->symbols[*ip];
        (*ip)++;
    }

    if (i == MAX_NAME_LEN) {
        LOG(ERROR, "Too long name error\n");
        return SYNTAX_ERR;
    }

    op_t func = is_operator(name);
    if (func != POISON) {
        node->type = OP;
        node->value = func;

        node->parent = nullptr;
        node->right = nullptr;
        node->left = nullptr;
    }
    else {
        node->type = VAR;
        node->value = index_in_nametable(name);

        node->parent = nullptr;
        node->right = nullptr;
        node->left = nullptr;
    }
    return NO_ERR;
}

double prog_tree_t::index_in_nametable(char* name) {
    assert(name != nullptr);

    double index = find_name_in_nametable(name);

    if (index >= 0) {
        return index;
    }
    return add_name_to_nametable(name);
}

double prog_tree_t::add_name_to_nametable(char* name) {
    assert(name != nullptr);

    strncpy(var_nametable_[var_nametable_size_++].name, name, MAX_NAME_LEN);
    return var_nametable_size_ - 1;
}

double prog_tree_t::find_name_in_nametable(char* name) {
    assert(name != nullptr);

    for (size_t i = 0; i < var_nametable_size_; i++) {
        if (strncmp(var_nametable_[i].name, name, MAX_NAME_LEN) == 0) {
            return i;
        }
    }
    return -1;
}

void prog_tree_t::print_var_nametable() {
    for (size_t i = 0; i < var_nametable_size_; i++) {
        printf("name[%zu]: %s\n", i, var_nametable_[i].name);
    }
}

op_t prog_tree_t::is_operator(char* op) {
    assert(op != nullptr);

    for (size_t i = 0; i < func_name_table_len; i++) {
        if (strncmp(func_name_table[i].name, op, sizeof(func_name_table[i].name)) == 0) {
            return func_name_table[i].code;
        }
    }
    return POISON;
}

void prog_tree_t::initialize_op_node(node_t* node, double val) {
    assert(node != nullptr);

    node->parent = nullptr;
    node->right = nullptr;
    node->left = nullptr;

    node->type = OP;
    node->value = val;
}

void prog_tree_t::parse_number(text_t* text, size_t* ip, node_t* node) {
    assert(text != nullptr);
    assert(ip != nullptr);

    node->parent = nullptr;
    node->right = nullptr;
    node->left = nullptr;
    node->type = NUM;

    int num = 0;
    while (text->symbols[*ip] >= '0' && text->symbols[*ip] <= '9') {
        if (num > INT_MAX / 10) {
            LOG(ERROR, "SYNTAX ERROR: Num is greater than MAX_INT");
            return;
        }
        num = (num * 10) + (text->symbols[*ip] - '0');
        (*ip)++;
    }

    if (text->symbols[*ip] != '.') {
        node->value = (double) num;
        return;
    }
    (*ip)++;

    int dec = 0;
    while (text->symbols[*ip] >= '0' && text->symbols[*ip] <= '9') {
        if (num > INT_MAX / 10) {
            LOG(ERROR, "SYNTAX ERROR: Num is greater than MAX_INT");
            return;
        }

        dec++;
        num = (num * 10) + text->symbols[*ip] - '0';
        (*ip)++;
    }

    node->value = (dec == 0) ? (double) num : (double) num / dec;
    return;
}
