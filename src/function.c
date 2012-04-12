#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "treeoperation.h"
#include "variable.h"
#include "function.h"

cons_t* g_functions;

cons_t* define_func(cons_t* definition)
{
    cons_t* func_name = NULL;
    cons_t* old_func = get_func(g_functions, definition);
    if(old_func){
        free_tree(old_func->cdr);
        free(old_func->svalue);
        *old_func = *definition;
        func_name = copy_cell(definition);
        definition->cdr = NULL;
        definition->svalue = NULL;
        free_tree(definition);
    }else{
        cons_t* function_cell = create_cons_cell(definition, LIST);
        if(g_functions == NULL){
            g_functions = function_cell;
        }else{
            cons_t* head = g_functions;
            while(head->cdr != NULL){
                head = head->cdr;
            }
            head->cdr = function_cell;
        }
        func_name = copy_cell(definition);
    }
    return func_name;
}

cons_t* get_func(cons_t* functions, cons_t* name)
{
    cons_t* func;
   
    if(functions == NULL){
        return NULL;
    }

    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);

    for(func = functions; func != NULL; func = func->cdr){
        assert(func->type == LIST && func->car->type == STR);
        if(strcmp(name->svalue, func->car->svalue) == 0){
            return func->car;
        }
    }
    return NULL; 
}

