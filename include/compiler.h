#ifndef COMPILER_H
#define COMPILER_H

struct lisp_t;
struct cons_t;
struct lisp_mn_t;

struct lisp_mn_t* lisp_compile(struct lisp_t* L, struct cons_t* tree);
void lisp_printcode(struct lisp_mn_t* code);

#endif

