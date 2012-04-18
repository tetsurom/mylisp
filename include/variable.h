#ifndef VARIABLE_H
#define VARIABLE_H

typedef struct cons_t cons_t;
typedef struct stack_t stack_t;

void get_var(cons_t* vars, stack_t* stack);
void set_variable(cons_t* vars, stack_t* stack);

#endif

