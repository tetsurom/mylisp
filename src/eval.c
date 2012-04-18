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

static void lisp_call(lisp_t* L, cons_t* function_cell, int argc, cons_t* sp_funcparam);

void lisp_eval(lisp_t* L, cons_t* tree, cons_t* sp_funcparam)
{
    cons_t* vars = L->g_variables;
    switch(tree->type){
    case LIST:
    {
        cons_t* head = tree->car;
        int argc = 0;
        switch(head->type){
        case SETQ:
        {
            head = head->cdr;
            stack_push(L->g_stack, head);
            ++argc;
            for(head = head->cdr; head; head = head->cdr){
                lisp_eval(L, head, sp_funcparam);
                ++argc;
            }
            set_variable(vars, L->g_stack);
            break;
        }
        case DEFUN:
        {
            stack_push(L->g_stack, head->cdr);
            define_func(L, head->cdr);
            ((cons_t*)stack_top(L->g_stack))->cdr = NULL;
            break;
        }
        case IF:
        {
            cons_t* condition;
            head = head->cdr;
            lisp_eval(L, head, sp_funcparam);
            condition = (cons_t*)stack_top(L->g_stack);
            if(condition->type != NIL){
                head = head->cdr;
            }else{
                head = head->cdr->cdr;
            }
            stack_pop(L->g_stack); // pop condition
            lisp_eval(L, head, sp_funcparam);
            ((cons_t*)stack_top(L->g_stack))->cdr = NULL;
            break;
        }
        default:
            for(head = head->cdr; head; head = head->cdr){
                lisp_eval(L, head, sp_funcparam);
                ++argc;
            }
            assert(L->g_stack->top == top_before + argc);
            lisp_call(L, tree->car, argc, sp_funcparam);
            break;
        }
        break;
    }
    case STR:
        stack_push(L->g_stack, tree);
        while(((cons_t*)stack_get(L->g_stack, -1))->type == STR){
            get_var(vars, L->g_stack);
        }
        break;
    case PARAM:
    {
        //int param_top = sp_funcparam;
        //cons_t* p = (cons_t*)stack_get(L->g_stack, sp_funcparam + tree->iValue);
        stack_push(L->g_stack, sp_funcparam + tree->iValue);
        break;
    }
    default:
        stack_push(L->g_stack, tree);
        break;
    }
    assert(L->g_stack->top == top_before + 1);
}

static void lisp_call(lisp_t* L, cons_t* function_cell, int argc, cons_t* sp_funcparam)
{
    stack_t* stack = L->g_stack;
    char* function = function_cell->svalue;
    int top = stack->top;
    int is_operator = function_cell->type >= OP_ADD && function_cell->type <= OP_GEQ;
    

    if(is_operator){
        int i;
        cons_t ret_cell;
        cons_t* temp;
        int type = function_cell->type;
        ret_cell.cdr = NULL;
        ret_cell.type = TRUE;

        temp = (cons_t*)stack_get(stack, -argc);
        assert(temp->type == INT);
        ret_cell.iValue = temp->iValue;

        for(i = 1; i < argc; ++i){
            int rhs;
            ++temp;// = (cons_t*)stack_get(stack, -argc + i);
            assert(temp->type == INT);
            rhs = temp->iValue;
            switch(type){
            case OP_ADD:
                ret_cell.iValue += rhs;
                ret_cell.type = INT;
                break;
            case OP_SUB:
                ret_cell.iValue -= rhs;
                ret_cell.type = INT;
                break;
            case OP_DIV:
                ret_cell.iValue /= rhs;
                ret_cell.type = INT;
                break;
            case OP_MUL:
                ret_cell.iValue *= rhs;
                ret_cell.type = INT;
                break;
            case OP_L:
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue < rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case OP_G:
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue > rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case OP_LEQ:
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue <= rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case OP_GEQ:
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue >= rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case OP_EQ:
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue == rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            }
        }
        stack_settop(stack, top - argc);
        stack_push(stack, (void*)&ret_cell);
    }else{
        cons_t* func = get_func(L, function);
        if(func){
            cons_t* proc = func->cdr->cdr;
            int param_top = stack->top - argc + 1;
            cons_t* param_top_cell = (cons_t*)stack_get(stack, param_top);
            lisp_eval(L, proc, param_top_cell);
            if(argc > 0){
                *param_top_cell = *((cons_t*)stack_top(stack));
            }
            stack_settop(stack, param_top);
        }
    }
}


