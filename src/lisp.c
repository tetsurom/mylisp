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
#include "eval.h"
#include "istack.h"
#include "lisp.h"
#include "util.h"

void lisp_clearsymbols(lisp_t* L);

lisp_t* lisp_open()
{
    lisp_t* L = ALLOC(lisp_t);
    L->g_stack = istack_create(1024);
    L->g_variables = NULL;
    L->g_functions = NULL;
    L->g_symbols = NULL;
    return L;
}

void lisp_close(lisp_t* L)
{
    lisp_clear_functions(L);
    lisp_clear_variables(L);
    lisp_clearsymbols(L);
    istack_destroy(L->g_stack);
    free(L);
}

char* lisp_addsymbol(lisp_t* L, const char* symbol)
{
    lisp_list_t* list;
    for(list = L->g_symbols; list && list->next; list = list->next){
        if(strcmp(symbol, list->name) == 0){
            return list->name;
        }
    }
    if(list){
        list = list->next = ALLOC(lisp_list_t);
    }else{
        list = L->g_symbols = ALLOC(lisp_list_t);
    }
    list->name = strdup(symbol);
    list->address = NULL;
    list->next = NULL;
    return list->name;
}

void lisp_clearsymbols(lisp_t* L)
{
    lisp_list_t* symbol;
    lisp_list_t* next;
    for(symbol = L->g_symbols; symbol != NULL; symbol = next){
        next = symbol->next;
        free(symbol->name);
        free(symbol);
    }
}

