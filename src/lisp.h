#ifndef LISP_H
#define LISP_H

typedef struct cons_t cons_t;
typedef struct stack_t stack_t;

typedef struct lisp_t {
    stack_t* g_stack;
    cons_t* g_variables;
    cons_t* g_functions;
} lisp_t;

lisp_t* lisp_open();
void lisp_close(lisp_t* L);

#endif

