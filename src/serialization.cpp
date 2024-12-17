#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "prog_tree.h"
#include "logger.h"

void prog_tree_t::print_node_r(FILE* ostream, node_t* node, size_t tab_cnt) {
    if (node == nullptr) {
        return;
    }

    for (size_t i = 0; i < tab_cnt; i++) {
        fprintf(ostream, "\t");
    }

    fprintf(ostream, "{");

    print_node_data(ostream, node);

    if (node->left != nullptr) {
        fprintf(ostream, "\n");
        print_node_r(ostream, node->left, tab_cnt + 1);
    }

    if (node->right != nullptr) {
        fprintf(ostream, "\n");
        print_node_r(ostream, node->right, tab_cnt + 1);
    }

    if (node->right != nullptr || node->left != nullptr) {
        for (size_t i = 0; i < tab_cnt; i++) {
            fprintf(ostream, "\t");
        }
    }

    fprintf(ostream, "}\n");
}

void prog_tree_t::deserialization(FILE* ostream) {
    // NOTE -  print header
    print_node_r(ostream, root_, 0);
}


void prog_tree_t::print_node_data(FILE* ostream, node_t* node) {
    assert(ostream != nullptr);
    if (node == nullptr) return;

    switch ((int) node->type) {
        case NUM:
            fprintf(ostream,"NUM: \"%lg\" ", node->value);
            break;
        case OP:
            fprintf(ostream, "OP: \"");
            print_operator(ostream, node->value);
            fprintf(ostream, "\" ");
            break;
        case FUNC:
            fprintf(ostream,  "FUNC: %s ", var_nametable_[(int) node->value].name);
            break;
        case VAR:
            fprintf(ostream,  "VAR: %s ", var_nametable_[(int) node->value].name);
            break;
        default:
            break;
    }
}

void prog_tree_t::serialization(FILE* istream) {
    assert(istream != nullptr);

    text_t text = {};
    if (text_ctor(&text, istream) != TEXT_NO_ERRORS) {
        LOG(ERROR, "Failed to read text\n");
        return;
    }

    size_t ip = 0;
    root_ = parse_node_r(&text, &ip);
    text_dtor(&text);
}

void prog_tree_t::skip_spaces(text_t* text, size_t* ip) {
    assert(text != nullptr);
    assert(ip != nullptr);

    while (isspace(text->symbols[*ip])) {
        (*ip)++;
    }
}

node_t* prog_tree_t::parse_node_r(text_t* text, size_t* ip) {
    assert(text != nullptr);
    assert(ip != nullptr);

    char token[MAX_NAME_LEN] = "";
    size_t _ip = 0;

    skip_spaces(text, ip);

    if (*ip == text->symbols_amount) {
        return nullptr;
    }

    if (text->symbols[*ip] == '{') {
        (*ip)++;
    }

    while (!(text->symbols[*ip] == '{' || text->symbols[*ip] == '}')) {
        if (_ip < MAX_NAME_LEN - 1) {
            token[_ip++] = (char) text->symbols[(*ip)++];
        }
        else {
            LOG(ERROR, "Token %s is too long\n", token);
            return nullptr;
        }
        skip_spaces(text, ip);
    }
    token[_ip] = '\0';

    node_t* node = nullptr;
    if (strstr(token, "OP") != nullptr) {
        node = new_node(OP, parse_operator(token));
    }
    else if (strstr(token, "FUNC") != nullptr) {
        node = new_node(FUNC, parse_func(token));
    }
    else if (strstr(token, "VAR") != nullptr) {
        node = new_node(VAR, parse_variable(token));
    }
    else if (strstr(token, "NUM") != nullptr) {
        node = new_node(NUM, parse_number(token));
    }

    if (node == nullptr) {
        LOG(ERROR, "node is null\n");
        return nullptr;
    }

    skip_spaces(text, ip);

    if (text->symbols[*ip] == '{') {
        node->left = parse_node_r(text, ip);
        if (node->left != nullptr) {
            node->left->parent = node;
        }
    }
    else {
        node->left = nullptr;
    }

    skip_spaces(text, ip);

    if (text->symbols[*ip] == '{') {
            node->right = parse_node_r(text, ip);
            if (node->right != nullptr) {
                node->right->parent = node;
            }
        }
    else {
        node->right = nullptr;
    }

    skip_spaces(text, ip);

    if (text->symbols[*ip] != '}') {
        LOG(ERROR, "Syntax err %s, ip = %zu\n", text->symbols[*ip], *ip);
    }
    else {
        (*ip)++;
    }
    return node;
}

double prog_tree_t::parse_number(char* buffer) {
    char* exp_end_ptr = strstr(buffer, "\""); // FIXME :
    char* real = nullptr;

    double val = strtod(strstr(buffer, "\"") + 1, &real);
    return val;
}

double prog_tree_t::parse_operator(char* buffer) {
    for (size_t i = 0; i < func_name_table_len; i++) {
        if (strstr(buffer, func_name_table[i].name) != nullptr) {
            return func_name_table[i].code;
        }
        if (strstr(buffer, "IAEQ") != nullptr) {  //NOTE - fixme babe
            return IAEQ;
        }

    }
    return NULL;
}

double prog_tree_t::parse_func(char* buffer) {
    char name[MAX_NAME_LEN] = "";
    size_t j = 0;

    while (isspace(buffer[j])) {
        j++;
    }

    buffer = strstr(buffer, ":") + 1;
    for (size_t i = 0; i < MAX_NAME_LEN; i++) {
        if (!isdigit(buffer[j]) && !isalpha(buffer[j]) && buffer[j] != '_') {
            name[i] = '\0';
            break;
        }
        name[i] = (char) buffer[j++];
    }

    return index_in_nametable(name);
}

double prog_tree_t:: parse_variable(char* buffer) {
    return parse_func(buffer);
}

node_t* prog_tree_t::new_node(type_t type, double val) {
    node_t* new_node = (node_t*) calloc(sizeof(node_t), 1);
    if (new_node == nullptr) {
        LOG(ERROR, "Mem alloc err\n");
        return nullptr;
    }

    new_node->type = type;
    new_node->value = val;

    new_node->right = nullptr;
    new_node->left = nullptr;
    new_node->parent = nullptr;
    return new_node;
}

