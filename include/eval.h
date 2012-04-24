#ifndef EVAL_H
#define EVAL_H

typedef struct cons_t cons_t;
typedef struct lisp_t lisp_t;

void lisp_eval(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam);

#endif

