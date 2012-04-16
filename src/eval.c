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

void lisp_eval(lisp_t* L, cons_t* tree, int sp_funcparam)
{
    cons_t* vars = L->g_variables;
    int top_before = L->g_stack->top;
    switch(tree->type){
    case LIST:
    {
        cons_t* head = tree->car;
        int argc = 0;
        assert(head->type == STR);
        stack_push(L->g_stack, head);
        if(strcmp(head->svalue, "setq") == 0){
            head = head->cdr;
            stack_push(L->g_stack, head);
            ++argc;
        }
        if(strcmp(head->svalue, "defun") == 0){
            stack_pop(L->g_stack); // pop "defun"
            stack_push(L->g_stack, head->cdr);
            define_func(L, head->cdr);
            ((cons_t*)stack_top(L->g_stack))->cdr = NULL;
            break;
        }
        if(strcmp(head->svalue, "if") == 0){
            cons_t* condition;
            head = head->cdr;
            stack_pop(L->g_stack); // pop "if"
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
            assert(L->g_stack->top == top_before + 1);
            break;
        }
        for(head = head->cdr; head; head = head->cdr){
            lisp_eval(L, head, sp_funcparam);
            ++argc;
        }
        assert(L->g_stack->top == top_before + argc + 1);
        lisp_call(L, argc, sp_funcparam);
        assert(L->g_stack->top == top_before + 1);
        break;
    }
    case STR:
        stack_push(L->g_stack, tree);
        while(((cons_t*)stack_get(L->g_stack, -1))->type == STR){
            get_var(vars, L->g_stack);
        }
        assert(L->g_stack->top == top_before + 1);
        break;
    case PARAM:
    {
        int param_top = sp_funcparam + 1;
        cons_t* p = (cons_t*)stack_get(L->g_stack, param_top + tree->iValue);
        stack_push(L->g_stack, p);
        assert(L->g_stack->top == top_before + 1);
        break;
    }
    default:
        stack_push(L->g_stack, tree);
        assert(L->g_stack->top == top_before + 1);
        break;
    }
    assert(L->g_stack->top == top_before + 1);
}

void lisp_call(lisp_t* L, int argc, int sp_funcparam)
{
    stack_t* stack = L->g_stack;
    cons_t* function_cell = (cons_t*)stack_get(stack, -argc - 1);
    char* function = function_cell->svalue;
    int is_operator = 0;
    int top = stack->top;
    if(strlen(function) == 1){
        switch(function[0]){
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
            is_operator = 1;
            break;
        default:
            break;
        }
    }
    if(is_operator){
        int i;
        cons_t ret_cell;
        cons_t* temp;
        ret_cell.cdr = NULL;
        ret_cell.type = TRUE;

        temp = (cons_t*)stack_get(stack, -argc);
        assert(temp->type == INT);
        ret_cell.iValue = temp->iValue;

        for(i = 1; i < argc; ++i){
            int rhs;
            temp = (cons_t*)stack_get(stack, -argc + i);
            assert(temp->type == INT);
            rhs = temp->iValue;
            switch(function[0]){
            case '+':
                ret_cell.iValue += rhs;
                ret_cell.type = INT;
                break;
            case '-':
                ret_cell.iValue -= rhs;
                ret_cell.type = INT;
                break;
            case '/':
                ret_cell.iValue /= rhs;
                ret_cell.type = INT;
                break;
            case '*':
                ret_cell.iValue *= rhs;
                ret_cell.type = INT;
                break;
            case '<':
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue < rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case '>':
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue > rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            case '=':
                if(ret_cell.type == TRUE){
                    ret_cell.type = ret_cell.iValue == rhs ? TRUE : NIL;
                    ret_cell.iValue = rhs;
                }else{
                    ret_cell.type = NIL;
                }
                break;
            }
        }
        stack_settop(stack, top - argc - 1);
        stack_push(stack, (void*)&ret_cell);
    }
    if(strcmp(function, "setq") == 0){
        set_variable(L->g_variables, stack);
        stack_remove(stack, -2);
    }
    {
        cons_t* func = get_func(L, function);
        if(func){
            cons_t* proc = func->cdr->cdr;
            lisp_eval(L, proc, stack->top - argc);
            *((cons_t*)stack_get(stack, top - argc)) = *((cons_t*)stack_top(stack)); 
            stack_settop(stack, top - argc);
        }
    }
}

cons_t* eval(cons_t* tree, cons_t* vars, stack_t* stack)
{
    const char* op = NULL;
    op = tree->svalue;



    if(strcmp(op, "if") == 0){
        cons_t* condition = NULL;
        cons_t* on_true = NULL;
        cons_t* on_nil = NULL;
        cons_t* ret_cell = NULL;

        condition = tree->cdr;
        assert(condition != NULL);
        on_true = condition->cdr;
        assert(on_true != NULL);
        on_nil = on_true->cdr;
        assert(on_nil != NULL);


        if(condition->type == NIL){
            if(on_nil->type == LIST){
                ret_cell = eval(on_nil->car, vars, stack);
                on_nil->car = NULL; 
            }else{
                ret_cell = copy_cell(on_nil);
            }
        }else{
            if(on_true->type == LIST){
                ret_cell = eval(on_true->car, vars, stack);
                on_true->car = NULL;
            }else{
                ret_cell = copy_cell(on_true);
            }
        }

        free_tree(tree);
        return ret_cell;
    }

    free_tree(tree);
    return create_cons_cell(NULL, NIL);
}


