#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "function.h"
#include "lisp.h"
#include "util.h"

void define_func(lisp_t* L, const char* name, lisp_mn_t* definition, int argc)
{
    lisp_func_t* func;
    lisp_func_t** regist_point = &L->g_functions;

    for(func = L->g_functions; func != NULL; func = func->next){
        regist_point = &(func->next);
        if(strcmp(name, func->name) == 0){
            free(func->address);
            func->address = definition;
            func->argc = argc;
            //printf("function %s [%p]\n", name, func);
            return;
        }
    }
    func = (*regist_point) = ALLOC(lisp_func_t);
    func->address = definition;
    func->name = lisp_addsymbol(L, name);
    func->argc = argc;
    func->next = NULL;
    if(definition){
        //printf("function %s [%p]\n", name, func);
    }
}

lisp_func_t* get_func(lisp_t* L, const char* name)
{
    lisp_func_t* func = NULL;
    assert(name != NULL);
    for(func = L->g_functions; func != NULL; func = func->next){
        if(strcmp(name, func->name) == 0){
            return func;
        }
    }
    return NULL; 
}

void lisp_clear_functions(lisp_t* L)
{
    lisp_func_t* func;
    for(func = L->g_functions; func != NULL;){
        lisp_func_t* next = func->next;
        free(func->address);
        free(func);
        func = next;
    }
}

