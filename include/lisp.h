#ifndef LISP_H
#define LISP_H

struct cons_t;
struct istack_t;

typedef struct lisp_list_t {
    char* name;
    struct lisp_mn_t* address;
    struct lisp_list_t* next;   
} lisp_list_t;

typedef struct lisp_func_t {
    char* name;
    struct lisp_mn_t* address;
    struct lisp_func_t* next;
    int argc;
} lisp_func_t;

typedef struct lisp_t {
    struct istack_t* g_stack;
    struct lisp_list_t* g_variables;
    struct lisp_func_t* g_functions;
    struct lisp_list_t* g_symbols;
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
        LC_ADDC,
        LC_SUBC,
        LC_MULC,
        LC_DIVC,
        LC_EQC,
        LC_LSC,
        LC_GRC,
        LC_LSEQC,
        LC_GREQC,
        LC_NEQC,
        LC_RET,
        LC_END,
    } opcode;
    union {
        int ioperand;
        void* poperand;
    };
} lisp_mn_t;

struct lisp_t* lisp_open();
void lisp_close(struct lisp_t* L);
char* lisp_addsymbol(struct lisp_t* L, const char* symbol);

#endif

