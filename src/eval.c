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
    //if(tree->type == LIST){
        lisp_mn_t* code = lisp_compile(L, tree);
        //putchar('\n');
        if(code){
            lisp_printcode(code);
            lisp_postprocess(code);
            lisp_execute(L, code, NULL);
            printf("----------------\n");
            int val = istack_top(L->g_stack);
            if(val == 0){
                printf("nil\n");
            }else if(val == 1){
                printf("t\n");
            }else{
                printf("%d\n", val);
            }
            istack_settop(L->g_stack, 0);
            free(code);
        }
    //}
}

#define OP_2STACK(top, op) { *(top-1) op ## = *top; top -= 1; }
#define OP_2STACKCMP(top, op) { *(top-1) = *(top-1) op *top; top -= 1; }
#define PUSH(top, v) { *(++top) = v; }

#define LABELP(x) (&&L_ ## x)
#define CASE(x) L_ ## x
#define JUMP(x) goto *x

void** jumptable;
static void lisp_execute(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam);

void** getJumpTable()
{
    lisp_execute(NULL, NULL, NULL);
    return jumptable;
}

static void lisp_execute(lisp_t* L, lisp_mn_t* code_mn, int* sp_funcparam)
{
    static void* jtable[] = {
        LABELP(LC_PUSH),
        LABELP(LC_LOADP),
        LABELP(LC_LOADV),
        LABELP(LC_LOADVS),
        LABELP(LC_CALL),
        LABELP(LC_CALLS),
        LABELP(LC_SETQ),
        LABELP(LC_IFNIL),
        LABELP(LC_JUMP),
        LABELP(LC_ADD),
        LABELP(LC_SUB),
        LABELP(LC_MUL),
        LABELP(LC_DIV),
        LABELP(LC_EQ),
        LABELP(LC_LS),
        LABELP(LC_GR),
        LABELP(LC_LSEQ),
        LABELP(LC_GREQ),
        LABELP(LC_NEQ),
        LABELP(LC_ADDC),
        LABELP(LC_SUBC),
        LABELP(LC_MULC),
        LABELP(LC_DIVC),
        LABELP(LC_EQC),
        LABELP(LC_LSC),
        LABELP(LC_GRC),
        LABELP(LC_LSEQC),
        LABELP(LC_GREQC),
        LABELP(LC_NEQC),
        LABELP(LC_INC),
        LABELP(LC_DEC),
        LABELP(LC_DEC2),
        LABELP(LC_RET),
        LABELP(LC_RETV),
        LABELP(LC_NOP),
    };

    if(!code_mn){
        jumptable = jtable;    
        return;
    }
    lisp_mn_t* code_head = code_mn;
    istack_t* stack = L->g_stack;
    int* data = stack->data;
    int* top = stack->data + stack->top;

    JUMP(code_mn->opcode);
    
    CASE(LC_PUSH):
        PUSH(top, code_mn->ioperand);
        JUMP((++code_mn)->opcode);
    CASE(LC_LOADP):
        PUSH(top, sp_funcparam[code_mn->ioperand]);
        JUMP((++code_mn)->opcode);
    CASE(LC_LOADVS):
    {
        int* var = lisp_get_var(L, (char*)code_mn->poperand); 
        if(var){
            code_mn->opcode = LC_LOADV;
            code_mn->poperand = var;
        }else{
            fprintf(stderr, "ERROR: function %s is undefined.\n", (char*)code_mn->poperand);
            PUSH(top, 0);
            JUMP((++code_mn)->opcode);
        }
    }
    CASE(LC_LOADV):
        PUSH(top, *(int*)code_mn->poperand);
        JUMP((++code_mn)->opcode);
    CASE(LC_CALLS):
    {
        lisp_func_t* function = get_func(L, (char*)code_mn->poperand); 
        if(function){
            code_mn->opcode = LC_CALL;
            code_mn->poperand = function;
        }else{
            fprintf(stderr, "ERROR: function %s is undefined.\n", (char*)code_mn->poperand);
            JUMP((++code_mn)->opcode);
        }
    }
    CASE(LC_CALL):
    {
        lisp_func_t* function = (lisp_func_t*)code_mn->poperand;
        int argc = function->argc;
        int* param_top = top - argc + 1;
        stack->top = (int)(top - data);
        lisp_execute(L, function->address, param_top);
        *param_top = data[stack->top];
        top = param_top;
        JUMP((++code_mn)->opcode);
    }
    CASE(LC_SETQ):
        *(int*)code_mn->poperand = *top;
        JUMP((++code_mn)->opcode);
    CASE(LC_IFNIL):
        JUMP( (*(top--) ? (++code_mn) : (code_mn = code_head + code_mn->ioperand))->opcode );
    CASE(LC_JUMP):
        code_mn = code_head + code_mn->ioperand;
        JUMP(code_mn->opcode);
    CASE(LC_ADD):
        OP_2STACK(top, +);
        JUMP((++code_mn)->opcode);
    CASE(LC_SUB):
        OP_2STACK(top, -);
        JUMP((++code_mn)->opcode);
    CASE(LC_MUL):
        OP_2STACK(top, *);
        JUMP((++code_mn)->opcode);
    CASE(LC_DIV):
        OP_2STACK(top, /);
        JUMP((++code_mn)->opcode);
    CASE(LC_EQ):
        OP_2STACKCMP(top, ==);
        JUMP((++code_mn)->opcode);
    CASE(LC_LS):
        OP_2STACKCMP(top, <);
        JUMP((++code_mn)->opcode);
    CASE(LC_GR):
        OP_2STACKCMP(top, >);
        JUMP((++code_mn)->opcode);
    CASE(LC_LSEQ):
        OP_2STACKCMP(top, <=);
        JUMP((++code_mn)->opcode);
    CASE(LC_GREQ):
        OP_2STACKCMP(top, >=);
        JUMP((++code_mn)->opcode);
    CASE(LC_NEQ):
        OP_2STACKCMP(top, !=);
        JUMP((++code_mn)->opcode);
    CASE(LC_ADDC):
        *top += code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_SUBC):
        *top -= code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_MULC):
        *top *= code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_DIVC):
        *top /= code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_EQC):
        *top = *top == code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_LSC):
        *top = *top < code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_GRC):
        *top = *top > code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_LSEQC):
        *top = *top <= code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_GREQC):
        *top = *top <= code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_NEQC):
        *top = *top != code_mn->ioperand;
        JUMP((++code_mn)->opcode);
    CASE(LC_INC):
        *top += 1;
        JUMP((++code_mn)->opcode);
    CASE(LC_DEC):
        *top -= 1;
        JUMP((++code_mn)->opcode);
    CASE(LC_DEC2):
        *top -= 2;
        JUMP((++code_mn)->opcode);
    CASE(LC_RETV):
        PUSH(top, code_mn->ioperand);
    CASE(LC_RET):
        stack->top = (int)(top - data);
        return;
    CASE(LC_NOP):
        JUMP((++code_mn)->opcode);
}

