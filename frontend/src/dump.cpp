#include <errno.h>
#include <string.h>
#include <assert.h>
#include "prog_tree.h"
#include "logger.h"

const size_t MAX_FILENAME_LEN = 40;
const size_t COMMAND_SIZE = 100;

const char* FILENAME = "prog_tree";

static FILE** get_dump_ostream() {
    static FILE* file = nullptr;
    return &file;
}

void prog_tree_t::set_dump_ostream(FILE* ostream) {
    *get_dump_ostream() = ostream;
}

//=========================================================================================

void prog_tree_t::print_preorder_() {
    print_preorder(root_);
}

void prog_tree_t::print_inorder_() {
    print_inorder(stdout, root_, 0);
}

//=========================================================================================

void prog_tree_t::dump_tree() {
    dump(root_);
}

void prog_tree_t::print_preorder(node_t* node) {
    if (node == nullptr) {
        return;
    }

    printf("(");

    switch (node->type) {
        case OP: {
            print_operator(stdout, node->value);
            break;
        }
        case FUNC:
        case VAR: {
            printf("%s", var_nametable_[(int) node->value].name);
            break;
        }
        case NUM: {
            printf("%f", node->value);
            break;
        }
        default:
            break;
    };

    if (node->left != nullptr) {
        print_preorder(node->left);
    }
    if (node->right != nullptr) {
        print_preorder(node->right);
    }

    printf(")");
}

void prog_tree_t::print_inorder(FILE* ostream, node_t* node, int parent_precedence) {
    if (node == nullptr) {
        return;
    }

    int current_precedence = 0;

    switch (node->type) {
        case OP:
            current_precedence = get_operator_precedence((int) node->value);
            break;
        case VAR:
        case FUNC:
        case NUM:
            current_precedence = -1;
            break;
        default:
            break;
    }

    if (node->left != nullptr && !(
       (int) node->type  == OP     && (
       (int) node->value == DIV    ||
       (int) node->value == LN     ||
       (int) node->value == LOG    ||
       (int) node->value == SIN    ||
       (int) node->value == COS    ||
       (int) node->value == TG     ||
       (int) node->value == CTG    ||
       (int) node->value == SH     ||
       (int) node->value == CH     ||
       (int) node->value == TH     ||
       (int) node->value == CTH    ||
       (int) node->value == ARCSIN ||
       (int) node->value == ARCCOS ||
       (int) node->value == ARCTG  ||
       (int) node->value == ARCCTG ||
       (int) node->value == ARCCH  ||
       (int) node->value == ARCSH  ||
       (int) node->value == ARCTH  ||
       (int) node->value == ARCCTH))) {
        if (current_precedence > parent_precedence) {
            fprintf(ostream, "(");
        }

        print_inorder(ostream, node->left, current_precedence);

        if (current_precedence > parent_precedence) {
            fprintf(ostream, ")");
        }
    }

    switch (node->type) {
        case OP: {
            switch ((int) node->value)  {
                case DIV: {
                    fprintf(ostream, "\\frac{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}{");
                    print_inorder(ostream, node->right, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case LN: {
                    fprintf(ostream, "\\ln{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case LOG: {
                    fprintf(ostream, "\\log{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case SIN: {
                    fprintf(ostream, "\\sin{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case COS: {
                    fprintf(ostream, "\\cos{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case TG: {
                    fprintf(ostream, "\\tan{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case CTG: {
                    fprintf(ostream, "\\cot{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case SH: {
                    fprintf(ostream, "\\sinh{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case CH: {
                    fprintf(ostream, "\\cosh{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case TH: {
                    fprintf(ostream, "\\tanh{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case CTH: {
                    fprintf(ostream, "\\coth{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case ARCSIN: {
                    fprintf(ostream, "\\arcsin{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case ARCCOS: {
                    fprintf(ostream, "\\arccos{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case ARCTG: {
                    fprintf(ostream, "\\arctan{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                case ARCCTG: {
                    fprintf(ostream, "\\arccot{");
                    print_inorder(ostream, node->left, current_precedence);
                    fprintf(ostream, "}");
                    break;
                }
                default:
                    print_operator(ostream, node->value);
                    break;
            }
            break;
        }
        case FUNC:
        case VAR: {
            fprintf(ostream, "%s", var_nametable_[(int) node->value].name);
            break;
        }
        case NUM: {
            fprintf(ostream, "%g", node->value);
            break;
        }
        default:
            break;
    };

    if (node->right != nullptr && !((int) node->type == OP && is_unary(node->value))) {
        if (node->right == nullptr) return;
        if (current_precedence > parent_precedence) {
            fprintf(ostream, "(");
        }

        print_inorder(ostream, node->right, current_precedence);

        if (current_precedence > parent_precedence) {
            fprintf(ostream, ")");
        }
    }
}

bool prog_tree_t::is_unary(double value) {
    return (int) value == DIV    ||
           (int) value == LN     ||
           (int) value == LOG    ||
           (int) value == SIN    ||
           (int) value == COS    ||
           (int) value == TG     ||
           (int) value == CTG    ||
           (int) value == SH     ||
           (int) value == CH     ||
           (int) value == TH     ||
           (int) value == CTH    ||
           (int) value == ARCSIN ||
           (int) value == ARCCOS ||
           (int) value == ARCTG  ||
           (int) value == ARCCTG ||
           (int) value == ARCCH  ||
           (int) value == ARCSH  ||
           (int) value == ARCTH  ||
           (int) value == ARCCTH;
}

int prog_tree_t::get_operator_precedence(int operation) {
    switch (operation) {
        case ADD:
        case SUB:
            return 1;
        case MUL:
        case DIV:
            return 2;
        case POW:
            return 3;
        default:
            return 0;
    }
}

void prog_tree_t::print_tree_to_tex(FILE* ostream, node_t* root) {
    print_to_tex(ostream , root);
}

void prog_tree_t::print_to_tex(FILE* ostream, node_t* node) {
    static bool first_dump = true;
    if (first_dump == true) {
        fprintf(ostream, "\\documentclass{article}\n"
                         "\\author{Alina Palonskaya}\n"
                         "\\date{November 2024}\n"
                         "\\begin{document}\n"
                         "\\maketitle\n");
    }

    print_exp_to_tex(ostream, node);

    first_dump = false;
}

void prog_tree_t::print_exp_to_tex(FILE* ostream, node_t* node) {
    assert(ostream != nullptr);
    assert(node != nullptr);

    fprintf(ostream, "$ ");
    print_inorder(ostream, node, 0);
    fprintf(ostream, " $\n\n");
}

//=========================================================================================

void prog_tree_t::dump(node_t* root) {
    assert(root != nullptr);

    FILE* ostream = *get_dump_ostream();
    if (ostream == nullptr) {
        LOG(ERROR, "Dump ostream is nullptr, print to stdout\n");
        ostream = stdout;
    }

    fprintf(ostream, "<pre>");
    static size_t image_cnt = 0;

    char tree_filename[MAX_FILENAME_LEN] = {};
    char image_filename[MAX_FILENAME_LEN] = {};

    snprintf(tree_filename, MAX_FILENAME_LEN, "data/images/%s%zu.dot", FILENAME, image_cnt);
    snprintf(image_filename, MAX_FILENAME_LEN, "data/images/%s%zu.png", FILENAME, image_cnt);

    FILE* tree_file = fopen(tree_filename, "wb");
    if (tree_file == nullptr) {
        LOG(ERROR, "Failed to open an output file\n");
        return;
    }

    printf_tree_dot_file(tree_file, root);

    if (fclose(tree_file) == EOF) {
        LOG(ERROR, "Failed to close a %s file\n" STRERROR(errno), tree_filename);
        return;
    }

    char command[COMMAND_SIZE] = "";

    snprintf(command, sizeof(command), "dot -Tpng %s -o %s", tree_filename, image_filename);

    if (system(command) != 0) {
        LOG(ERROR, "Failed to create an image\n");
        return;
    }

    fprintf(ostream, "\n<img src = \"../%s\" width = 50%%>\n", image_filename);
    image_cnt++;
}

void prog_tree_t::printf_tree_dot_file(FILE* tree_file, node_t* node) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    fprintf(tree_file, "digraph G {\n\t"
                       "rankdir=TB;\n\t"
                       "bgcolor=\"#DDA0DD\";\n\t"
                       "splines=true;\n\t"
                       "node [shape=box, width=1, height=0.5, style=filled, bgcolor=\"#DDA0DD\"];\n\t");

    print_nodes(tree_file, node, 1);
    print_links(tree_file, node);

    fprintf(tree_file, "}\n");
}

void prog_tree_t::print_nodes(FILE* tree_file, node_t* node, size_t rank) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    fprintf(tree_file, "node%zu [label=<<table border='0' cellspacing='0' bgcolor=", (size_t) node % 10000);

    switch (node->type) {
        case OP:
            if ((int) node->value == DECL) {
                fprintf(tree_file, "'#F804B7'");
                break;
            }
            if ((int) node->value == SEMICOLON) {
                fprintf(tree_file, "'#A80100'");
                break;
            }
            fprintf(tree_file, "'#F8C4B7'");
            break;
        case VAR:
            fprintf(tree_file, "'#B7F8CA  '");
            break;
        case FUNC:
            fprintf(tree_file, "'#FFAAFF  '");
            break;
        case NUM:
            fprintf(tree_file, "'#ADD8E6'");
            break;
        default:
            break;
    };

    fprintf(tree_file,
            "> <tr><td>addr: %p</td></tr>"
            "<tr><td bgcolor='black' height='1'></td></tr><tr><td>", node);

    switch (node->type) {
        case OP: {
            print_operator(tree_file, node->value);
            break;
        }
        case VAR:
            fprintf(tree_file,  "%s", var_nametable_[(int) node->value].name);
            break;
        case NUM:
            fprintf(tree_file, "%f", node->value);
            break;
        case FUNC:
            fprintf(tree_file,  "%s", var_nametable_[(int) node->value].name);
            break;
        default:
            break;
    };

    fprintf(tree_file, "</td></tr><tr><td bgcolor='black' height='1'></td></tr>"
                       "<tr><td>right = %p</td></tr>"
                       "<tr><td bgcolor='black' height='1'></td></tr>"
                       "<tr><td>left = %p</td></tr>"
                        "<tr><td bgcolor='black' height='1'></td></tr>"
                       "<tr><td>parent = %p</td></tr>"
                       "</table>>];\n\t"
                       "rank = %zu\n", node->right, node->left, node->parent, rank);

    if (node->left != nullptr) print_nodes(tree_file, node->left, rank + 1);
    if (node->right != nullptr) print_nodes(tree_file, node->right, rank + 1);
}

void prog_tree_t::print_links(FILE* tree_file, node_t* node) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    if (node->left != nullptr) {
        fprintf(tree_file, "node%zu -> node%zu [weight=10,color=\"black\"];\n\t",
                           (size_t) node % 10000, (size_t) node->left % 10000);
        print_links(tree_file, node->left);
    }
    if (node->right != nullptr) {
        fprintf(tree_file, "node%zu -> node%zu [weight=10,color=\"black\"];\n\t",
                           (size_t) node % 10000, (size_t) node->right % 10000);
        print_links(tree_file, node->right);
    }
}

//=========================================================================================

void prog_tree_t::print_operator(FILE* ostream, double value) {
    assert(ostream != nullptr);

    for (size_t i = 0; i < func_name_table_len; i++) {
        if ((int) value == IA) {
            fprintf(ostream, " IA ");
            return;
        }
        else if ((int) value == IAEQ) {
            fprintf(ostream, " IAEQ ");
            return;
        }
        else if ((int) value == IB) {
            fprintf(ostream, " IB ");
            return;
        }
        else if ((int) value == IBEQ) {
            fprintf(ostream, " IBEQ ");
            return;
        }
        else if (func_name_table[i].code == (int) value) {
            fprintf(ostream, " %s ", func_name_table[i].name);
            return;
        }
    }
    LOG(ERROR, "Unknown sign %f was detected\n", value);
}

//======================================================================================

int prog_tree_t::def_operator(char* op) {
    if (strnstr(op, "+", MAX_OP_LEN) != nullptr) {
        return ADD;
    }
    else if (strnstr(op, "-", MAX_OP_LEN) != nullptr) {
        return SUB;
    }
    else if (strnstr(op, "/", MAX_OP_LEN) != nullptr) {
        return DIV;
    }
    else if (strnstr(op, "*", MAX_OP_LEN) != nullptr) {
        return MUL;
    }
    else if (strnstr(op, "^", MAX_OP_LEN) != nullptr) {
        return POW;
    }
    else if (strnstr(op, "log", MAX_OP_LEN) != nullptr) {
        return LOG;
    }
    else if (strnstr(op, "ln", MAX_OP_LEN) != nullptr) {
        return LN;
    }
    else if (strnstr(op, "exp", MAX_OP_LEN) != nullptr) {
        return EXP;
    }
    else if (strnstr(op, "arcsin", MAX_OP_LEN) != nullptr) {
        return ARCSIN;
    }
    else if (strnstr(op, "arccos", MAX_OP_LEN) != nullptr) {
        return ARCCOS;
    }
    else if (strnstr(op, "arctg", MAX_OP_LEN) != nullptr) {
        return ARCTG;
    }
    else if (strnstr(op, "arcctg", MAX_OP_LEN) != nullptr) {
        return ARCCTG;
    }
    else if (strnstr(op, "arcsh", MAX_OP_LEN) != nullptr) {
        return ARCSH;
    }
    else if (strnstr(op, "arcch", MAX_OP_LEN) != nullptr) {
        return ARCCH;
    }
    else if (strnstr(op, "arcth", MAX_OP_LEN) != nullptr) {
        return ARCTH;
    }
    else if (strnstr(op, "arccth", MAX_OP_LEN) != nullptr) {
        return ARCCTH;
    }
    else if (strnstr(op, "sin", MAX_OP_LEN) != nullptr) {
        return SIN;
    }
    else if (strnstr(op, "cos", MAX_OP_LEN) != nullptr) {
        return COS;
    }
    else if (strnstr(op, "ctg", MAX_OP_LEN) != nullptr) {
        return CTG;
    }
    else if (strnstr(op, "tg", MAX_OP_LEN) != nullptr) {
        return TG;
    }
    else if (strnstr(op, "sh", MAX_OP_LEN) != nullptr) {
        return SH;
    }
    else if (strnstr(op, "ch", MAX_OP_LEN) != nullptr) {
        return CH;
    }
    else if (strnstr(op, "cth", MAX_OP_LEN) != nullptr) {
        return CTH;
    }
    else if (strnstr(op, "th", MAX_OP_LEN) != nullptr) {
        return TH;
    }
    else {
        LOG(ERROR, "Unknown operation %s was detected\n", op);
        return -1;
    }
}


