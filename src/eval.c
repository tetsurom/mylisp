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

cons_t* eval_all(cons_t* head, cons_t* vars, stack_t* stack)
{
    while(head != NULL){
        cons_t* next = head->cdr;
        if(head->type == LIST){
            eval_car_and_replace(head, vars, stack);
        }
        head = next;
    }
    return head;
}

cons_t* eval_car_and_replace(cons_t* tree, cons_t* vars, stack_t* stack)
{
    assert(tree->type == LIST && tree->car != NULL);
    cons_t* cdr = tree->cdr;
    cons_t* ret = eval(tree->car, vars, stack);
    *tree = *ret;
    tree->cdr = cdr;
    ret->car = NULL;
    free_tree(ret);
    return tree;
}

cons_t* eval(cons_t* tree, cons_t* vars, stack_t* stack)
{
    const char* op = NULL;
    int is_operator = 0;
   
    if(tree->type == LIST){
        eval(tree->car, vars, stack);
        //eval_all(tree, vars, stack); 
        return tree;
    }else if(tree->type != STR){
        return tree;
    }

    op = tree->svalue;

    if(strlen(op) == 1){
        switch(op[0]){
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
        int top = stack->top;
        int argc = 0;
        int i;
        cons_t ret_cell;
        cons_t* head;
        cons_t* temp;
        ret_cell.cdr = NULL;
        ret_cell.type = TRUE;

        for(head = tree->cdr; head; head = head->cdr){
            if(head->type == LIST){
                eval(head->car, vars, stack);
            }else{
                stack_push(stack, head);
                /*
                while(((cons_t*)stack_get(stack, -1))->type == STR){
                    get_var(vars, stack);
                }
                */
            }
            ++argc;
        }
               
        temp = (cons_t*)stack_get(stack, -argc);
        assert(temp->type == INT);
        ret_cell.iValue = temp->iValue;

        for(i = 1; i < argc; ++i){
            int rhs;
            temp = (cons_t*)stack_get(stack, -argc + i);
            assert(temp->type == INT);
            rhs = temp->iValue;
            switch(op[0]){
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
        stack_settop(stack, top);
        stack_push(stack, (void*)&ret_cell);
        return NULL;
    }

    if(strcmp(op, "defun") == 0){
        cons_t* ret_cell = define_func(tree->cdr);
        tree->cdr = NULL;
        free_tree(tree);
        return ret_cell;
    }
    if(strcmp(op, "setq") == 0){
        cons_t* ret_cell = NULL;
        set_variable(vars, stack);
        tree->cdr = NULL;
        free_tree(tree);
        return ret_cell;
    }
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

        if(condition->type == LIST){
            eval_car_and_replace(condition, vars, stack);
        }

        if(on_true->type == STR && vars != NULL){
            //get_var_and_replace(vars, on_true);
        }
        if(on_nil->type == STR && vars != NULL){
            //get_var_and_replace(vars, on_nil);
        }

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

    {
        cons_t* func = get_func(g_functions, tree);
        if(func){
            cons_t* ret_cell;
            eval_all(tree->cdr, vars, stack);
            ret_cell = apply(func, tree->cdr, vars, stack);
            free_tree(tree);
            return ret_cell;
        }
    }

    if(vars){
        if(tree->cdr){
            free_tree(tree->cdr);
            tree->cdr = NULL;
        }
        return NULL;//get_var_and_replace(vars, tree);
    }

    free_tree(tree);
    return create_cons_cell(NULL, NIL);
}

cons_t* apply(cons_t* function, cons_t* args, cons_t* upper_vars, stack_t* stack)
{
    assert(function->type == STR);
    
    cons_t* vars = NULL;
    cons_t* ret = NULL;
    
    cons_t* params = function->cdr->car;
    cons_t* proc = function->cdr->cdr->car;

    vars = create_cons_cell(params, LIST);
    vars->cdr = create_cons_cell(args, LIST);
    vars->cdr->cdr = upper_vars;
    
    ret = eval(copy_tree(proc), vars, stack);
    
    vars->cdr->cdr = NULL;
    vars->cdr->car = NULL;
    vars->car = NULL;
   
    free_tree(vars);
    
    return ret;
}


