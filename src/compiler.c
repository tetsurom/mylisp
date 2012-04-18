#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "compiler.h"
#include "treeoperation.h"


static int getParamOrder(cons_t* params, cons_t* name)
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

static void bindParam(cons_t* params, cons_t* tree)
{
    if(tree == NULL){
        return;
    }
    
    if(tree->type == STR && tree->svalue != NULL){
        int order = getParamOrder(params, tree);
        if(order >= 0){
            free(tree->svalue);
            tree->type = PARAM;
            tree->iValue = order;
        }
    }
    if(tree->type == LIST){
        bindParam(params, tree->car);
    }
    if(tree->cdr != NULL){
        bindParam(params, tree->cdr);
    }
}

void lisp_compile(lisp_t* L, cons_t* tree)
{
    switch(tree->type){
    case LIST:
    {
        cons_t* func = tree->car;
        cons_t* param = func->cdr;
        switch(func->type){
        case DEFUN:
        {
            cons_t* args = param->cdr->car;
            cons_t* proc = param->cdr->cdr;
            bindParam(args, proc);
            printf("function %s (", param->svalue);
            print_tree(args);
            printf("):\n");
            lisp_compile(L, proc);
            printf("ret\n");
            printf("----------------\n");
            break;
        }
        case SETQ:
            break;
        case IF:
        {
            cons_t* on_true = param->cdr;
            cons_t* on_nil = on_true->cdr;
            lisp_compile(L, param);
            printf("ifnil N:");
            putchar('\n');
            printf("T:\n");
            lisp_compile(L, on_true);
            printf("goto ENDIF:\n");
            printf("N:\n");
            lisp_compile(L, on_nil);
            printf("ENDIF:\n");
            break;
        }
        default:
        {
            int is_op = func->type != STR;
            lisp_compile(L, param);
            for(param = param->cdr; param; param = param->cdr){
                lisp_compile(L, param); 
                if(is_op){
                    print_cell(func);
                    putchar('\n');
                }
            }
            if(!is_op){
                printf("call ");
                print_cell(func);
                putchar('\n');
            }
            break;
        }
        }
        break;
    }
    case PARAM:
        printf("loadp ");
        print_cell(tree);
        putchar('\n');
        break;
    case STR:
        printf("loadv ");
        print_cell(tree);
        putchar('\n');
        break;
    case INT:
        printf("push ");
        print_cell(tree);
        putchar('\n');
        break;
    default:
        printf("ERROR\n");
        break;
    }
}

