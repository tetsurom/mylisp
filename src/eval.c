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

#define STACK_TOP(x) (x->data + x->top)
#define STACK_OP1(x) (x->data[x->top - 1])
#define STACK_OP2(x) (x->data[x->top])
#define STACK_RET(x) (x->data[x->top - 1])

void lisp_eval(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam)
{
    lisp_mn_t* code_head = code_mn;
    istack_t* stack = L->g_stack;
    assert(code_mn);
    for(;; ++code_mn){
        register int stack_top = stack->top;
        int* data = stack->data;
        int* stack_top_p = data + stack_top;
        switch(code_mn->opcode){
        case LC_PUSH:
            data[++stack->top] = code_mn->ioperand;
            break;
        case LC_LOADP:
            data[++stack->top] = sp_funcparam[code_mn->ioperand];
            break;
        case LC_LOADV:
            istack_push(stack, *(int*)code_mn->poperand);
            break;
        case LC_LOADVS:
            break;
        case LC_CALL:
        {
            lisp_func_t* function = (lisp_func_t*)code_mn->poperand;
            int argc = function->argc;
            int param_top_index = stack_top - argc + 1;
            int* param_top = stack->data + param_top_index;
            lisp_eval(L, function->address, param_top);
            if(argc > 0){
                data[param_top_index] = data[stack->top];
                stack->top = param_top_index;
            }
            break;
        }
        case LC_CALLS:
            break;
        case LC_SETQ:
            break;
        case LC_IFNIL:
            if(!data[stack->top--]){
                code_mn = code_head + code_mn->ioperand - 1;
            }
            break;
        case LC_JUMP:
            code_mn = code_head + code_mn->ioperand - 1;
            break;
        case LC_ADD:
            //op2 = istack_popget(stack);
            //op1 = istack_popget(stack);
            //istack_push(stack, op1 + op2);
            *(stack_top_p - 1) += *stack_top_p;
            break;
        case LC_SUB:
            data[--stack->top] -= data[stack_top];
            break;
        case LC_MUL:
            data[stack_top-1] *= data[stack_top];
            --stack->top;
            break;
        case LC_DIV:
            data[stack_top-1] /= data[stack_top];
            --stack->top;
            break;
        case LC_EQ:
            data[stack_top-1] = data[stack_top-1] == data[stack_top];
            --stack->top;
            break;
        case LC_LS:
            data[stack_top-1] = data[stack_top-1] < data[stack_top];
            --stack->top;
            break;
        case LC_GR:
            data[stack_top-1] = data[stack_top-1] > data[stack_top];
            --stack->top;
            break;
        case LC_LSEQ:
            data[stack_top-1] = data[stack_top-1] <= data[stack_top];
            --stack->top;
            break;
        case LC_GREQ:
            data[stack_top-1] = data[stack_top-1] <= data[stack_top];
            --stack->top;
            break;
        case LC_NEQ:
            data[stack_top-1] = data[stack_top-1] != data[stack_top];
            --stack->top;
            break;
        case LC_ADDC:
            data[stack_top] += code_mn->ioperand;
            break;
        case LC_SUBC:
            *stack_top_p -= code_mn->ioperand;
            break;
        case LC_MULC:
            *STACK_TOP(stack) *= code_mn->ioperand;
            break;
        case LC_DIVC:
            *STACK_TOP(stack) /= code_mn->ioperand;
            break;
        case LC_EQC:
            *STACK_TOP(stack) = *STACK_TOP(stack) == code_mn->ioperand;
            break;
        case LC_LSC:
            data[stack_top] = data[stack_top] < code_mn->ioperand;
            break;
        case LC_GRC:
            *STACK_TOP(stack) = *STACK_TOP(stack) > code_mn->ioperand;
            break;
        case LC_LSEQC:
            *STACK_TOP(stack) = *STACK_TOP(stack) <= code_mn->ioperand;
            break;
        case LC_GREQC:
            *STACK_TOP(stack) = *STACK_TOP(stack) <= code_mn->ioperand;
            break;
        case LC_NEQC:
            *STACK_TOP(stack) = *STACK_TOP(stack) != code_mn->ioperand;
            break;
        case LC_RET:
            return;
        default:
            break;
        }
    }
}

