#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "variable.h"
#include "stack.h"
#include "lisp.h"
#include "util.h"

void lisp_set_var(lisp_t* L, const char* name, int definition)
{
    lisp_list_t* var;
    lisp_list_t** regist_point = &L->g_variables;

    for(var = L->g_variables; var != NULL; var = var->next){
        regist_point = &(var->next);
        if(strcmp(name, var->name) == 0){
            *(int*)(var->address) = definition;
            printf("variable %s [%p]\n", name, var);
            return;
        }
    }
    var = (*regist_point) = ALLOC(lisp_list_t);
    var->address = (lisp_mn_t*)ALLOC(int);
    *(int*)var->address = definition;
    var->name = lisp_addsymbol(L, name);
    var->next = NULL;
    if(definition){
        printf("variable %s [%p]\n", name, var);
    }
}

int* lisp_get_var(lisp_t* L, const char* name)
{
    lisp_list_t* var = NULL;
    assert(name != NULL);
    for(var = L->g_variables; var != NULL; var = var->next){
        if(strcmp(name, var->name) == 0){
            return (int*)var->address;
        }
    }
    return NULL; 
}

void lisp_clear_variables(lisp_t* L)
{
    lisp_list_t* var;
    for(var = L->g_variables; var != NULL;){
        lisp_list_t* next = var->next;
        free(var->address);
        free(var);
        var = next;
    }
}
