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

void get_var(cons_t* vars, stack_t* stack)
{
    cons_t* name = NULL;
    cons_t* names = NULL;
    cons_t* values = NULL;
    cons_t* upper_vars = NULL;
    assert(vars != NULL);

    names = vars->car;
    values = vars->cdr->car;
    upper_vars = vars->cdr->cdr;

    name = (cons_t*)stack_top(stack);

    assert(name != NULL);
    assert(name->type == STR);

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
    {
        cons_t nil_cell;
        nil_cell.type = NIL;
        nil_cell.car = NULL;
        nil_cell.cdr = NULL;
        stack_pop(stack);
        stack_push(stack, &nil_cell);
    }
}

void set_variable(cons_t* vars, stack_t* stack)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    cons_t* name = NULL;
    cons_t* value = NULL;

    assert(vars != NULL);
   
    name = (cons_t*)stack_get(stack, -2);
    value = (cons_t*)stack_get(stack, -1);
    
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);
   

    if(vars->type == NIL){
        vars->type = LIST;
        vars->car = copy_cell(name);
        vars->cdr->type = LIST;
        vars->cdr->car = copy_cell(value); 
        stack_remove(stack, -2);
        return;
    }

    names = vars->car;
    values = vars->cdr->car;

    for(;;){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* cdr = values->cdr;
            if(values->type == STR){
                free(values->svalue);
            }else if(values->type == LIST){
                free_tree(values->car);
            }
            *values = *value;
            values->cdr = cdr;
            stack_remove(stack, -2);
            return;
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
    stack_remove(stack, -2);
}


