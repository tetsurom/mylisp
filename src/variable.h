#ifndef VARIABLE_H
#define VARIABLE_H
#include "treeoperation.h"

cons_t* get_var(cons_t* vars, cons_t* name);
cons_t* get_var_and_replace(cons_t* vars, cons_t* name);
cons_t* set_variable(cons_t* vars, cons_t* name, cons_t* value);

extern cons_t* g_variables;

#endif

