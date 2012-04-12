#ifndef FUNCTION_H
#define FUNCTION_H

#include "treeoperation.h"

cons_t* define_func(cons_t* definition);
cons_t* get_func(cons_t* functions, cons_t* name);

extern cons_t* g_functions;

#endif

