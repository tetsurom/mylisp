#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "treeoperation.h"
#include "variable.h"
#include "function.h"
#include "compiler.h"
#include "eval.h"
#include "istack.h"
#include "lisp.h"

#define STACK_TOP(x) (x->data + x->top)
#define STACK_OP1(x) (x->data[x->top - 1])
#define STACK_OP2(x) (x->data[x->top])
#define STACK_RET(x) (x->data[x->top - 1])

static void lisp_execute(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam);

void lisp_eval(lisp_t* L, cons_t* tree)
{
    if(tree->type == LIST){
        lisp_mn_t* code = lisp_compile(L, tree);
        putchar('\n');
        if(code){
            lisp_printcode(code);
            lisp_execute(L, code, NULL);
            printf("----------------\n");
            printf("%d\n", istack_top(L->g_stack));
            istack_settop(L->g_stack, 0);
            free(code);
        }
    }
}

#define OP_2STACK(top, op) { *(top-1) op ## = *top; top -= 1; }
#define OP_2STACKCMP(top, op) { *(top-1) = *(top-1) op *top; top -= 1; }
#define PUSH(top, v) { *(++top) = v; }

static void lisp_execute(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam)
{
    lisp_mn_t* code_head = code_mn;
    istack_t* stack = L->g_stack;
    int* data = stack->data;
    int* top = stack->data + stack->top;

    assert(code_mn);
    for(;; ++code_mn){
        switch(code_mn->opcode){
        case LC_PUSH:
            PUSH(top, code_mn->ioperand);
            break;
        case LC_LOADP:
            PUSH(top, sp_funcparam[code_mn->ioperand]);
            break;
        case LC_LOADV:
            PUSH(top, *(int*)code_mn->poperand);
            break;
        case LC_LOADVS:
            break;
        case LC_CALL:
        {
            lisp_func_t* function = (lisp_func_t*)code_mn->poperand;
            int argc = function->argc;
            int* param_top = top - argc + 1;
            stack->top = (int)(top - data);
            //printf("%s(%i; %i, %i) ", function->name, function->argc, *param_top, *(param_top + 1));
            lisp_execute(L, function->address, param_top);
            //printf(" -> %i\n", data[stack->top]);
            if(argc > 0){
                *param_top = data[stack->top];
                top = param_top;
            }
            break;
        }
        case LC_CALLS:
            break;
        case LC_SETQ:
            break;
        case LC_IFNIL:
            if(!*(top--)){
                code_mn = code_head + code_mn->ioperand - 1;
            }
            break;
        case LC_JUMP:
            code_mn = code_head + code_mn->ioperand - 1;
            break;
        case LC_ADD:
            OP_2STACK(top, +);
            break;
        case LC_SUB:
            OP_2STACK(top, -);
            break;
        case LC_MUL:
            OP_2STACK(top, *);
            break;
        case LC_DIV:
            OP_2STACK(top, /);
            break;
        case LC_EQ:
            OP_2STACKCMP(top, ==);
            break;
        case LC_LS:
            OP_2STACKCMP(top, <);
            break;
        case LC_GR:
            OP_2STACKCMP(top, >);
            break;
        case LC_LSEQ:
            OP_2STACKCMP(top, <=);
            break;
        case LC_GREQ:
            OP_2STACKCMP(top, >=);
            break;
        case LC_NEQ:
            OP_2STACKCMP(top, !=);
            break;
        case LC_ADDC:
            *top += code_mn->ioperand;
            break;
        case LC_SUBC:
            *top -= code_mn->ioperand;
            break;
        case LC_MULC:
            *top *= code_mn->ioperand;
            break;
        case LC_DIVC:
            *top /= code_mn->ioperand;
            break;
        case LC_EQC:
            *top = *top == code_mn->ioperand;
            break;
        case LC_LSC:
            *top = *top < code_mn->ioperand;
            break;
        case LC_GRC:
            *top = *top > code_mn->ioperand;
            break;
        case LC_LSEQC:
            *top = *top <= code_mn->ioperand;
            break;
        case LC_GREQC:
            *top = *top <= code_mn->ioperand;
            break;
        case LC_NEQC:
            *top = *top != code_mn->ioperand;
            break;
        case LC_RET:
        case LC_END:
            stack->top = (int)(top - data);
            return;
        default:
            break;
        }
    }
}

