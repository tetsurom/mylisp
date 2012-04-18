#ifndef COMPILER_H
#define COMPILER_H

typedef struct lisp_t lisp_t;
typedef struct cons_t cons_t;

void lisp_compile(lisp_t* L, cons_t* tree);

#endif

