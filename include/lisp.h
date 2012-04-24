#ifndef LISP_H
#define LISP_H

typedef struct cons_t cons_t;
typedef struct istack_t istack_t;

typedef struct lisp_list_t {
    char* name;
    lisp_mn_t* address;
    struct lisp_list_t* next;   
} lisp_list_t;

typedef struct lisp_func_t {
    char* name;
    lisp_mn_t* address;
    struct lisp_func_t* next;
    int argc;
} lisp_func_t;

typedef struct lisp_t {
    istack_t* g_stack;
    lisp_list_t* g_variables;
    lisp_func_t* g_functions;
} lisp_t;

typedef struct lisp_mn_t{
    enum opcode_e {
        LC_PUSH,
        LC_LOADP,
        LC_LOADV,
        LC_LOADVS,
        LC_CALL,
        LC_CALLS,
        LC_SETQ,
        LC_IFNIL,
        LC_JUMP,
        LC_ADD,
        LC_SUB,
        LC_MUL,
        LC_DIV,
        LC_EQ,
        LC_LS,
        LC_GR,
        LC_LSEQ,
        LC_GREQ,
        LC_NEQ,
        LC_RET,
    } opcode;
    union {
        int ioperand;
        void* poperand;
    };
} lisp_mn_t;

lisp_t* lisp_open();
void lisp_close(lisp_t* L);

#endif

