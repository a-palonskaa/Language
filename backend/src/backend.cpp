#include <assert.h>
#include "backend.h"
#include "prog_tree.h"
#include "logger.h"

void backend_t::init(FILE* istream) {
    assert(istream != nullptr);

    prog_tree_.serialization(istream);
}

void backend_t::dtor() {
    prog_tree_.tree_dtor();
}

void backend_t::translate_to_asm(FILE* ostream) {
    assert(ostream != nullptr);

    fprintf(ostream, "push 0\n");
    fprintf(ostream, "pop hx\n");
    fprintf(ostream, "call main:\n");
    fprintf(ostream, "hlt\n");

    node_t* current_node = prog_tree_.root_;
    while (current_node != nullptr && current_node->left != nullptr) { //NOTE - if current_node->left->type == OP && current_node->left->type == OP) - syntax_err
        print_func(ostream, current_node->left);
        current_node = current_node->right;
    }

}

void backend_t::print_func(FILE* ostream, node_t* decl) {
    assert(ostream != nullptr);
    assert(decl != nullptr);
    size_t ram_index = 0; //FIXME -  suka huinya ebanay

    if (decl->left == nullptr || decl->left->type != OP || (int) decl->left->value != SPEC) {
        LOG(ERROR, "Syntax err at %p: %d %d\n", decl, decl->left->type == OP, (int) decl->left->value == SPEC);
        return;
    }
    if (decl->left->left->type != FUNC) {
        LOG(ERROR, "Syntax err at %p\n", decl);
        return;
    }

    fprintf(ostream, "%s:\n", prog_tree_.var_nametable_[(int) decl->left->left->value].name);
    fprintf(ostream, "push hx\n");
    fprintf(ostream, "push 100\n");
    fprintf(ostream, "add\n");
    fprintf(ostream, "pop hx\n");

    size_t i = 0;
    node_t* current_node = decl->left->right;
    while (1) {
        if (current_node == nullptr) break;
        if (current_node->type == VAR) {
            fprintf(ostream, "push %s\n", reg_by_num(i));
            fprintf(ostream, "pop [hx+%zu]\n", (size_t) current_node->value);
            break;
        }
        else if (current_node->type ==  OP && (int) current_node->value == SEMICOLON) {
            fprintf(ostream, "push %s\n", reg_by_num(i));
            fprintf(ostream, "pop [hx+%zu]\n", (size_t) current_node->left->value);
            current_node = current_node->right;
            i++;
        }
    }

    print_func_body(ostream, decl->right, ram_index);
}

const char* backend_t::reg_by_num(size_t i) {
    switch (i) {
        case 0: return "ax";
        case 1: return "bx";
        case 2: return "cx";
        case 3: return "dx";
        case 4: return "ex";
        case 5: return "fx";
        case 6: return "gx";
        default: return "";
    }
}

void backend_t::print_func_body(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);

    if (node == nullptr) {
        return;
    }

    node_t* my_node = node;
    while (my_node != nullptr && my_node->type == OP && (int) my_node->value == SEMICOLON) {
        print_expr(ostream, my_node->left, ram_index);
        my_node = my_node->right;
    }
}

// NOTE :
// EQ     // =
// IE     // ==
// INE    // !=
// IA     // >
// IAEQ   // >=
// IB     // <
// IBEQ   // <=
// IF
// ELSE
// WHILE
void backend_t::print_equal(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);
    if (node == nullptr) return;

    push_expr(ostream, node->right, ram_index);
    fprintf(ostream, "pop [hx+%zu]\n", (size_t) node->left->value);
}

void backend_t::print_expr(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);
    if (node == nullptr) return;

    switch ((int) node->value) {
        case DEF_VAR: {
            push_expr(ostream, node->left->right, ram_index); // FIXME - check errors
            fprintf(ostream, "pop [hx+%zu]\n", (size_t) node->left->left->value);
            break;
        }
        case CALL: {
            size_t i = 0;
            node_t* current_node = node->left;
            while (1) {
                if (current_node == nullptr) break;
                if (current_node->type ==  OP && (int) current_node->value == SEMICOLON) {
                    push_expr(ostream, current_node->left, ram_index);
                    fprintf(ostream, "pop %s\n", reg_by_num(i));
                    current_node = current_node->right;
                    i++;
                }
                else {
                    push_expr(ostream, current_node, ram_index);
                    fprintf(ostream, "pop %s\n", reg_by_num(i));
                    break;
                }
            }
            fprintf(ostream, "call %s:\n", prog_tree_.var_nametable_[(int) node->right->value].name);
            break;
        }
        case RETURN: {
            fprintf(ostream, "push hx\n");
            fprintf(ostream, "push 100\n");
            fprintf(ostream, "sub\n");
            fprintf(ostream, "pop hx\n");
            fprintf(ostream, "ret\n");
            break;
        }
        case IF: {
            break;
         //NOTE -
        }
        case SEMICOLON: {
            print_if_else(ostream, node, ram_index);
            break;
        }
        case EQ: {
            print_equal(ostream, node, ram_index);
            break;
        }
        case IN: {
            fprintf(ostream, "in\n");
            fprintf(ostream, "pop [hx+%zu]\n", (size_t) node->left->value);
            break;
        }
        case OUT: {
            fprintf(ostream, "push [hx+%zu]\n", (size_t) node->left->value);
            fprintf(ostream, "out\n");
            break;
        }
        default:
            break;
    }
}

void backend_t::print_if_else(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);
    if (node == nullptr) return;


    node_t* if_node = node->left;
    if (if_node == nullptr || if_node->type != OP || (int) if_node->value != IF) {
        LOG(ERROR, "Syntax err\n");
        return;
    }

    node_t* else_node = node->right;
    if (else_node == nullptr || else_node->type != OP || (int) else_node->value != ELSE) {
        LOG(ERROR, "Syntax err\n");
        return;
    }

    push_expr(ostream, if_node->left->left, ram_index);
    push_expr(ostream, if_node->left->right, ram_index);

    size_t num = prog_tree_.jmp_cnt_++;
    switch ((int) if_node->left->value) {
        case IAEQ:
            fprintf(ostream, "jae %zu:\n", num);
            break;
        case IB:
            fprintf(ostream, "jb %zu:\n", num);
            break;
        case IE:
            fprintf(ostream, "je %zu:\n", num);
            break;
        case INE:
            fprintf(ostream, "jne %zu:\n", num);
            break;
        case IA:
            fprintf(ostream, "ja %zu:\n", num);
            break;
        case IBEQ:
            fprintf(ostream, "jbe %zu:\n", num);
            break;
        default:
            break;
    }

    node_t* my_node = else_node->left;
    while (my_node != nullptr) {
        print_expr(ostream, my_node->left, ram_index);
        my_node = my_node->right;
    }

    fprintf(ostream, "jmp finish_%zu:\n", num);
    fprintf(ostream, "%zu:\n", num);

    my_node = if_node->right;
    while (my_node != nullptr) {
        print_expr(ostream, my_node->left, ram_index);
        my_node = my_node->right;
    }

    fprintf(ostream, "finish_%zu:\n", num);
}

void backend_t::push_expr(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);
    if (node == nullptr) return;

    if (node->type == NUM) {
        fprintf(ostream, "push %lg\n", node->value);
    }
    else if (node->type == VAR) {
        fprintf(ostream, "push [hx+%zu]\n", (size_t) node->value);
    }
    else if (node->type == FUNC) {
        fprintf(ostream, "call %s:\n", prog_tree_.var_nametable_[(int)node->value].name);
    }
    else {
        print_op_expr_r(ostream, node, ram_index);
    }
}

void backend_t::print_op_expr_r(FILE* ostream, node_t* node, size_t ram_index) {
    assert(ostream != nullptr);
    if (node == nullptr) return;

    if (node->left != nullptr) {
        push_expr(ostream, node->left, ram_index);
    }

    if (node->right != nullptr) {
        push_expr(ostream, node->right, ram_index);
    }

    print_op(ostream, node, ram_index);
}

void backend_t::print_op(FILE* ostream, node_t* node, size_t ram_index) { //NOTE -  unsued ram
    switch ((int) node->value) {
        case ADD:
            fprintf(ostream, "add\n");
            break;
        case SUB:
            fprintf(ostream, "sub\n");
            break;
        case DIV:
            fprintf(ostream, "div\n");
            break;
        case MUL:
            fprintf(ostream, "mul\n");
            break;
        case POW:
            fprintf(ostream, "pow\n");
            break;
        case LOG:
            fprintf(ostream, "log\n");
            break;
        case LN:
            fprintf(ostream, "ln\n");
            break;
        case EXP:
            fprintf(ostream, "exp\n");
            break;
        case ARCSIN:
            fprintf(ostream, "arcsin\n");
            break;
        case ARCCOS:
            fprintf(ostream, "arccos\n");
            break;
        case ARCTG:
            fprintf(ostream, "arctg\n");
            break;
        case ARCCTG:
            fprintf(ostream, "arcctg\n");
            break;
        case ARCCH:
            fprintf(ostream, "arcch\n");
            break;
        case ARCSH:
            fprintf(ostream, "arcsh\n");
            break;
        case ARCTH:
            fprintf(ostream, "arcth\n");
            break;
        case ARCCTH:
            fprintf(ostream, "arccth\n");
            break;
        case SIN:
            fprintf(ostream, "sin\n");
            break;
        case COS:
            fprintf(ostream, "cos\n");
            break;
        case TG:
            fprintf(ostream, "tg\n");
            break;
        case CTG:
            fprintf(ostream, "ctg\n");
            break;
        case SH:
            fprintf(ostream, "sh\n");
            break;
        case CH:
            fprintf(ostream, "ch\n");
            break;
        case TH:
            fprintf(ostream, "th\n");
            break;
        case CTH:
            fprintf(ostream, "cth\n");
            break;

    }
}

void backend_t::set_dump_ostream(FILE* ostream) {
    prog_tree_.set_dump_ostream(ostream);
}

void backend_t::dump() {
    prog_tree_.dump(prog_tree_.root_);
}
