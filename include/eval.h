#ifndef EVAL_H
#define EVAL_H

struct cons_t;
struct lisp_t;

void lisp_eval(struct lisp_t* L, struct lisp_mn_t* code_mn, int* sp_funcparam);

#endif

