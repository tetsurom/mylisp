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

static void lisp_call(lisp_t* L, lisp_func_t* function);

void lisp_eval(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam)
{
    lisp_mn_t* code_head = code_mn;
    istack_t* stack = L->g_stack;
    int op1;
    int op2;
    assert(code_mn);
    --code_mn;
    do{
        int stack_top = stack->top;
        ++code_mn;
        switch(code_mn->opcode){
        case LC_PUSH:
            istack_push(stack, code_mn->ioperand);
            break;
        case LC_LOADP:
            istack_push(stack, sp_funcparam[code_mn->ioperand]);
            break;
        case LC_LOADV:
            istack_push(stack, *(int*)code_mn->poperand);
            break;
        case LC_LOADVS:
            break;
        case LC_CALL:
            lisp_call(L, (lisp_func_t*)code_mn->poperand);
            break;
        case LC_CALLS:
            break;
        case LC_SETQ:
            break;
        case LC_IFNIL:
            if(!istack_popget(stack)){
                code_mn = code_head + code_mn->ioperand - 1;
            }
            break;
        case LC_JUMP:
            code_mn = code_head + code_mn->ioperand - 1;
            break;
        case LC_ADD:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 + op2);
            break;
        case LC_SUB:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 - op2);
            break;
        case LC_MUL:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 * op2);
            break;
        case LC_DIV:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 / op2);
            break;
        case LC_EQ:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 == op2);
            break;
        case LC_LS:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 < op2);
            break;
        case LC_GR:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 > op2);
            break;
        case LC_LSEQ:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 <= op2);
            break;
        case LC_GREQ:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 >= op2);
            break;
        case LC_NEQ:
            op2 = istack_popget(stack);
            op1 = istack_popget(stack);
            istack_push(stack, op1 != op2);
            break;
        default:
            break;
        }
    }while(code_mn->opcode != LC_RET);
}

void lisp_call(lisp_t* L, lisp_func_t* function)
{
    istack_t* stack = L->g_stack;
    int stack_top = stack->top;
    int argc = function->argc;
    int param_top_index = stack->top - argc + 1;
    int* param_top = stack->data + param_top_index;
    lisp_eval(L, function->address, param_top);
    if(argc > 0){
        int ret = istack_top(stack);
        istack_settop(stack, param_top_index - 1);
        istack_push(stack, ret);
    }
    assert(stack->top == stack_top - argc + 1);
}


