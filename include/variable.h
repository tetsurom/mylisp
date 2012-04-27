#ifndef VARIABLE_H
#define VARIABLE_H

struct cons_t;
struct stack_t;

void get_var(struct cons_t* vars, struct stack_t* stack);
struct cons_t* get_var_and_replace(struct cons_t* vars, struct stack_t* stack);
void set_variable(struct cons_t* vars, struct stack_t* stack);

#endif

