#ifndef VARIABLE_H
#define VARIABLE_H

struct cons_t;
struct stack_t;

struct lisp_t;

void lisp_set_var(struct lisp_t* L, const char* name, int definition);
int* lisp_get_var(struct lisp_t* L, const char* name);
void lisp_clear_vars(struct lisp_t* L);

#endif

