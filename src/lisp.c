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
#include "stack.h"
#include "lisp.h"
#include "util.h"

lisp_t* lisp_open()
{
    lisp_t* L = ALLOC(lisp_t);
    L->g_stack = stack_create(sizeof(cons_t), 1024);
    //L->g_variables = create_cons_cell(NULL, NIL);
    //L->g_variables->cdr = create_cons_cell(NULL, NIL);
    L->g_functions = NULL;
    return L;
}

void lisp_close(lisp_t* L)
{
    if(L->g_functions){
        lisp_clear_functions(L);
    }
    if(L->g_variables){
        //free_tree(L->g_variables);
    }
    stack_destroy(L->g_stack);
    free(L);
}


