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
#include "lisp.h"

cons_t* define_func(lisp_t* L, cons_t* definition)
{
    cons_t* old_func = get_func(L, definition->svalue);
    if(old_func){
        *old_func = *definition;
    }else{
        cons_t* function_cell = create_cons_cell(definition, LIST);
        if(L->g_functions == NULL){
            L->g_functions = function_cell;
        }else{
            cons_t* head = L->g_functions;
            while(head->cdr != NULL){
                head = head->cdr;
            }
            head->cdr = function_cell;
        }
    }
    lisp_bindParam(definition->cdr->car, definition->cdr->cdr->car);
    return definition;
}

cons_t* get_func(lisp_t* L, const char* name)
{
    cons_t* func;
    cons_t* functions = L->g_functions;

    if(functions == NULL){
        return NULL;
    }

    assert(name != NULL);

    for(func = functions; func != NULL; func = func->cdr){
        assert(func->type == LIST && func->car->type == STR);
        if(strcmp(name, func->car->svalue) == 0){
            return func->car;
        }
    }
    return NULL; 
}

int lisp_getParamOrder(cons_t* params, cons_t* name)
{
    assert(name != NULL);
    assert(name->type == STR);
    int order = 0;

    while(params != NULL){
        if(strcmp(name->svalue, params->svalue) == 0){
            return order;
        }
        params = params->cdr;
        order++;
    }
    
    return -1;
}

void lisp_bindParam(cons_t* params, cons_t* tree)
{
    if(tree == NULL){
        return;
    }
    
    if(tree->type == STR && tree->svalue != NULL){
        int order = lisp_getParamOrder(params, tree);
        if(order >= 0){
            free(tree->svalue);
            tree->type = PARAM;
            tree->iValue = order;
        }
    }
    if(tree->type == LIST){
        lisp_bindParam(params, tree->car);
    }
    if(tree->cdr != NULL){
        lisp_bindParam(params, tree->cdr);
    }
}


