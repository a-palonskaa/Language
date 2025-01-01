#include <assert.h>
#include <math.h>
#include "middleend.h"
#include "prog_tree.h"
#include "logger.h"

void middleend_t::init(FILE* istream) {
    assert(istream != nullptr);

    prog_tree_.serialization(istream);
}

void middleend_t::dtor() {
    prog_tree_.tree_dtor();
}

void middleend_t::set_dump_ostream(FILE* ostream) {
    prog_tree_.set_dump_ostream(ostream);
}

void middleend_t::dump() {
    prog_tree_.dump(prog_tree_.root_);
}


node_t* middleend_t::optimize(node_t* node) {
    if (node == nullptr) {
        return nullptr;
    }

    bool change_flag = true;
    bool basic_op_flag = false;

    while (change_flag == true) {
        change_flag = false;
        basic_op_flag = false;
        change_flag |= calculations_optimization_r(node);
        node = basic_operations_optimization_r(node, ROOT, &basic_op_flag);
        change_flag |= basic_op_flag;
    }
    return node;
}

bool middleend_t::calculations_optimization_r(node_t* node) {
    if (node == nullptr) {
        return false;
    }

    bool change_flag = false;

    if (node->left != nullptr) {
        change_flag |= calculations_optimization_r(node->left);
    }

    if (node->right != nullptr) {
        change_flag |= calculations_optimization_r(node->right);
    }

    if ((node->type == OP) &&
       ((node->left  != nullptr && node->left->type  == NUM) &&
        (node->right != nullptr && node->right->type == NUM))) {

        if (node->right != nullptr && node->right->type != NUM) return false;
        if (node->left  != nullptr && node->left->type  != NUM) return false;

        if ((int) node->value == SUB && node->left == nullptr) return false;
        if ((int) node->value == ADD && node->left == nullptr) return false;

        node->value = calculate_value(node->value, node->left, node->right);

        free(node->left);
        free(node->right);

        node->left = nullptr;
        node->right = nullptr;
        node->type = NUM;
        change_flag |= true;
    }
    return change_flag;
}

double middleend_t::calculate_expression(node_t* node) {
    calculate_expression_r(node);
    return node->value;
};

void middleend_t::calculate_expression_r(node_t* node) {
    if (node == nullptr) {
        return;
    }

    if (node->left != nullptr) {
        calculate_expression_r(node->left);
    }

    if (node->right != nullptr) {
        calculate_expression_r(node->right);
    }

    if (node->type == OP && node->left != nullptr && node->right != nullptr) {
        double val_l = 0;
        double val_r = 0;

        if (node->left->type == VAR) {
            printf("Enter the value of '%s' variable: ", "x");
            scanf("%lf", &val_l);
        } else {
            val_l = node->left->value;
        }

        if (node->right->type == VAR) {
            printf("Enter the value of '%s' variable: ", "x");
            scanf("%lf", &val_r);
        } else {
            val_r = node->right->value;
        }

        node->value = calculate_value(node->value, node->left, node->right);

        free(node->left);
        free(node->right);

        node->left = nullptr;
        node->right = nullptr;
        node->type = NUM;
    }
}

double middleend_t::calculate_value(double op_type, node_t* node_l,  node_t* node_r) {
    double val_l = (node_l == nullptr) ? NAN : node_l->value;
    double val_r = (node_r == nullptr) ? NAN : node_r->value;

    switch ((int) op_type) {
        case ADD:
            return val_l + val_r;
        case SUB:
            return val_l - val_r;
        case MUL:
            return val_l * val_r;
        case DIV:
            return val_l / val_r;
        case POW:
            return pow(val_l, val_r);
        case SIN:
            return sin(val_r);
        case COS:
            return cos(val_r);
        case TG:
            return tan(val_r);
        case CTG:
            return 1 / tan(val_r);
        case SH:
            return sinh(val_r);
        case CH:
            return cosh(val_r);
        case TH:
            return tanh(val_r);
        case CTH:
            return 1 / tanh(val_r);
        case ARCSIN:
            return asin(val_r);
        case ARCCOS:
            return acos(val_r);
        case ARCTG:
            return atan(val_r);
        case ARCCTG:
            return M_PI / 2 - atan(val_r);
        case ARCSH:
            return asinh(val_r);
        case ARCCH:
            return acosh(val_r);
        case ARCTH:
            return atanh(val_r);
        case ARCCTH:
            return atanh(val_r);
        case LOG:
            return log(val_r) / log(val_l);
        case LN:
            return log(val_r);
        default:
            LOG(ERROR, "Undefined operation %d(%lf)\n", (int) op_type, op_type);
            return NAN;
            break;
    }
}

node_t* middleend_t::basic_operations_optimization_r(node_t* node, rel_t rel, bool* flag) {
    if (node == nullptr) {
        return nullptr;
    }

    if (node->left != nullptr) {
        basic_operations_optimization_r(node->left, LEFT, flag);
    }

    if (node->right != nullptr) {
        basic_operations_optimization_r(node->right, RIGHT, flag);
    }

    if (node->type == OP && node->left != nullptr && node->right != nullptr &&
       (node->left->type == NUM || node->right->type == NUM)) {
        if (node->left->type == NUM) {
            if ((int) node->left->value == 0) {
                node = null_val__optimization(node, LEFT, rel, flag);
            }
            else if ((int) node->left->value == 1) {
                node = one_val_optimization(node, LEFT, rel, flag);
            }
        }
        else if (node->right->type == NUM) {
            if ((int) node->right->value == 0) {
                node = null_val__optimization(node, RIGHT, rel, flag);
            }
            else if ((int) node->right->value == 1) {
                node = one_val_optimization(node, RIGHT, rel, flag);
            }
        }
    }
    return node;
}

node_t* middleend_t::null_val__optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag) {
    if (node == nullptr) {
        return nullptr;
    }

    switch ((int) node->value) {
        case ADD:
        case SUB:
            if (parent_rel == RIGHT) {
                *flag = true;
                if (rel == LEFT) {
                    node->parent->right = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->right = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == LEFT) {
                *flag = true;
                if (rel == LEFT) {
                    node->parent->left = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->left = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == ROOT) {
                *flag = true;
                if (rel == LEFT) {
                    node_t* new_root = node->right;
                    free(node->left);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
               else if (rel == RIGHT) {
                    node_t* new_root = node->left;
                    free(node->right);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
            }
            break;
        case DIV:
            if (rel == RIGHT) {
                LOG(ERROR, "Division by zero err\n");
                return nullptr;
            }

            [[fallthrough]];
        case MUL:
            *flag = true;
            prog_tree_.delete_subtree_r(node->right);
            prog_tree_.delete_subtree_r(node->left);

            node->right = nullptr;
            node->left = nullptr;

            node->type = NUM;
            node->value = 0;
            break;
        default:
            break;
    }
    return node;
}

node_t* middleend_t::one_val_optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag) {
    if (node == nullptr) {
        return nullptr;
    }

    switch ((int) node->value) {
        case MUL:
            *flag = true;
            if (parent_rel == LEFT) {
                if (rel == LEFT) {
                    node->parent->left = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->left = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == RIGHT) {
                if (rel == LEFT) {
                    node->parent->right = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->right = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == ROOT) {
                if (rel == LEFT) {
                    node_t* new_root = node->right;
                    free(node->left);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
                else if (rel == RIGHT) {
                    node_t* new_root = node->left;
                    free(node->right);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
            }
            break;
        case DIV:
            if (rel == LEFT) return node;
            *flag = true;
            if (parent_rel == LEFT) {
                node->parent->left = node->left;
            }
            else if (parent_rel == RIGHT) {
                node->parent->right = node->left;
            }
            else if (parent_rel == ROOT) {
                node_t* left = node->left;
                free(node);
                free(node->right);
                if (left == nullptr) return nullptr;
                left->parent = nullptr;
                return node->left;
            }
            free(node->right);
            free(node);
            break;
        case POW:
            if (rel == RIGHT) {
                *flag = true;
                if (parent_rel == LEFT) {
                    node->parent->left = node->left;
                }
                else if (parent_rel == RIGHT) {
                    node->parent->right = node->left;
                }

                free(node->right);
                free(node);
            }
            break;
        default:
            break;
    }
    return node;
}

void middleend_t::deserialization(FILE* ostream) {
    prog_tree_.deserialization(ostream);
}
