#ifndef TREE_OPERATION_H
#define TREE_OPERATION_H

enum cell_type_e {NIL, TRUE, INT, STR, LIST, PARAM, IF, SETQ, DEFUN,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_EQ, OP_L, OP_G, OP_LEQ, OP_GEQ};

typedef struct cons_t{
    int type;
    union {
        struct cons_t* car;
        int iValue;
        char* svalue;
    };
    struct cons_t* cdr;
} cons_t;

cons_t* create_cons_cell(void* value, int type);
void print_tree(cons_t* tree_head);
void free_tree(cons_t* tree_head);
cons_t* copy_cell(cons_t* cell);
cons_t* copy_tree(cons_t* tree_head);

#endif

