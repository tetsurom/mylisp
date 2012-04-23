#ifndef FUNCTION_H
#define FUNCTION_H

typedef struct cons_t cons_t;
typedef struct lisp_t lisp_t;
typedef struct lisp_mn_t lisp_mn_t;
typedef struct lisp_list_t lisp_list_t;

void define_func(lisp_t* L, const char* name, lisp_mn_t* definition);
lisp_list_t* get_func(lisp_t* L, const char* name);
void lisp_clear_functions(lisp_t* L);

#endif

