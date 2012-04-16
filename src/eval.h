#ifndef EVAL_H
#define EVAL_H

typedef struct cons_t cons_t;
typedef struct stack_t stack_t;
typedef struct lisp_t lisp_t;

void lisp_eval(lisp_t* L, cons_t* tree, int sp_funcparam);
void lisp_call(lisp_t* L, int argc, int sp_funcparam);

#endif

