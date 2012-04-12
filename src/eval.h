#ifndef EVAL_H
#define EVAL_H

typedef struct cons_t cons_t;
typedef struct stack stack;

cons_t* eval_all(cons_t* head, cons_t* vars, stack_t* stack);
cons_t* eval_car_and_replace(cons_t* tree, cons_t* vars, stack_t* stack);
cons_t* eval(cons_t* tree, cons_t* vars, stack_t* stack);
cons_t* apply(cons_t* function, cons_t* args, cons_t* upper_vars, stack_t* stack);


#endif

