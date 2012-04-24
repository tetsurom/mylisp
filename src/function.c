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
#include "util.h"

static char* strclone(const char* str)
{
    char* clone = CALLOC(char, strlen(str) + 1);
    strcpy(clone, str);
    return clone;
}

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
            printf("function %s [%p]\n", name, func);
            return;
        }
    }
    (*regist_point) = ALLOC(lisp_func_t);
    (**regist_point).address = definition;
    (**regist_point).name = strclone(name);
    (**regist_point).argc = argc;
    (**regist_point).next = NULL;
    printf("function %s [%p]\n", name, (*regist_point));
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

