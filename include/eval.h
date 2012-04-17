#ifndef EVAL_H
#define EVAL_H

typedef struct cons_t cons_t;
typedef struct lisp_t lisp_t;

void lisp_eval(lisp_t* L, cons_t* tree, int sp_funcparam);

#endif

