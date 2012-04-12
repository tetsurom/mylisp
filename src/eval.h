#ifndef EVAL_H
#define EVAL_H

#include "treeoperation.h"

cons_t* eval_all(cons_t* head, cons_t* vars);
cons_t* eval_car_and_replace(cons_t* tree, cons_t* vars);
cons_t* eval(cons_t* tree, cons_t* vars);
cons_t* apply(cons_t* function, cons_t* args, cons_t* upper_vars);


#endif

