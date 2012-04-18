#ifndef FUNCTION_H
#define FUNCTION_H

typedef struct cons_t cons_t;
typedef struct lisp_t lisp_t;

cons_t* define_func(lisp_t* L, cons_t* definition);
cons_t* get_func(lisp_t* L, const char* name);

#endif

