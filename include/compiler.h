#ifndef COMPILER_H
#define COMPILER_H

typedef struct lisp_t lisp_t;
typedef struct cons_t cons_t;
typedef struct lisp_mn_t lisp_mn_t;

lisp_mn_t* lisp_compile(lisp_t* L, cons_t* tree);
void lisp_printcode(lisp_mn_t* code);

#endif

