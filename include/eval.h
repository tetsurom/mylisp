#ifndef EVAL_H
#define EVAL_H

void lisp_eval(struct lisp_t* L, struct cons_t* tree);
void lisp_execute(struct lisp_t* L, struct lisp_mn_t* code_mn, int* sp_funcparam);

#endif

