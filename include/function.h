#ifndef FUNCTION_H
#define FUNCTION_H

struct cons_t;
struct lisp_t;
struct lisp_mn_t;
struct lisp_func_t;

void define_func(struct lisp_t* L, const char* name, struct lisp_mn_t* definition, int argc);
struct lisp_func_t* get_func(struct lisp_t* L, const char* name);
void lisp_clear_functions(struct lisp_t* L);

#endif

