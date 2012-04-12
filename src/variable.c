#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "variable.h"
#include "treeoperation.h"
#include "stack.h"

cons_t* g_variables;

cons_t NIL_CELL;

void get_var(cons_t* vars, stack_t* stack)
{
    cons_t* name = NULL;
    cons_t* names = NULL;
    cons_t* values = NULL;
    cons_t* upper_vars = NULL;
    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);

    names = vars->car;
    values = vars->cdr->car;
    upper_vars = vars->cdr->cdr;

    name = (cons_t*)stack_get(stack, -1);

    while(values != NULL){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            stack_pop(stack);
            stack_push(stack, values);
            ((cons_t*)stack_get(stack, -1))->cdr = NULL;
            return;
        }
        names = names->cdr;
        values = values->cdr;
    }
    if(upper_vars){
        get_var(upper_vars, stack);
        return;
    }
    stack_push(stack, &NIL_CELL);
}

cons_t* get_var_and_replace(cons_t* vars, stack_t* stack)
{
    /*
    assert(name->type == STR);
    cons_t* cdr = name->cdr;
    cons_t* ret = get_var(vars, name);
    free(name->svalue);
    *name = *ret;
    name->cdr = cdr;
    ret->car = NULL;
    free_tree(ret);
    */
    return NULL;
}

void set_variable(cons_t* vars, stack_t* stack)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    cons_t* name = NULL;
    cons_t* value = NULL;

    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);
   
    name = (cons_t*)stack_get(stack, -2);
    value = (cons_t*)stack_get(stack, -1);
 
    if(vars->type == NIL){
        vars->type = LIST;
        vars->car = copy_cell(name);
        vars->cdr->type = LIST;
        vars->cdr->car = copy_cell(value);
        *name = *value;
        stack_pop(stack);
        return;
    }

    names = vars->car;
    values = vars->cdr->car;

    for(;;){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* ret = copy_cell(value);
            cons_t* cdr = values->cdr;
            if(values->type == STR){
                free(values->svalue);
            }else if(values->type == LIST){
                free_tree(values->car);
            }
            *values = *value;
            values->cdr = cdr;
            *name = *value;
            stack_pop(stack);
        }
        if(names->cdr){
            assert(values->cdr);
            names = names->cdr;
            values = values->cdr;
        }else{
            break;
        }
    }

    values->cdr = copy_cell(value);
    names->cdr = copy_cell(name);
    *name = *value;
    stack_pop(stack);

    return;
}


