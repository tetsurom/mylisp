#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "variable.h"
#include "treeoperation.h"

cons_t* g_variables;

cons_t* get_var(cons_t* vars, cons_t* name)
{
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

    while(values != NULL){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* ret = copy_cell(values);
            return ret;
        }
        names = names->cdr;
        values = values->cdr;
    }
    if(upper_vars){
        return get_var(upper_vars, name);
    }
    return create_cons_cell(NULL, NIL);
}

cons_t* get_var_and_replace(cons_t* vars, cons_t* name)
{
    assert(name->type == STR);
    cons_t* cdr = name->cdr;
    cons_t* ret = get_var(vars, name);
    free(name->svalue);
    *name = *ret;
    name->cdr = cdr;
    ret->car = NULL;
    free_tree(ret);
    return name;
}

cons_t* set_variable(cons_t* vars, cons_t* name, cons_t* value)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    
    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);
    
    if(value->cdr){
        free_tree(value->cdr);
        value->cdr = NULL;
    }
    name->cdr = NULL;   
 
    if(vars->type == NIL){
        vars->type = LIST;
        vars->car = name;
        vars->cdr->type = LIST;
        vars->cdr->car = value;
        return copy_cell(value);
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
            value->svalue = NULL;
            free_tree(name);
            free_tree(value);
            return ret;
        }
        if(names->cdr){
            assert(values->cdr);
            names = names->cdr;
            values = values->cdr;
        }else{
            break;
        }
    }
    if(value->cdr){
        free_tree(value->cdr);
        value->cdr = NULL;
    }
    if(name->cdr){
        free_tree(name->cdr);
        name->cdr = NULL;
    }

    values->cdr = value;
    names->cdr = name;

    return copy_cell(value);
}


