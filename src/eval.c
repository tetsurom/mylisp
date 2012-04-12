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


cons_t* eval_all(cons_t* head, cons_t* vars)
{
    while(head != NULL){
        cons_t* next = head->cdr;
        if(head->type == LIST){
            eval_car_and_replace(head, vars);
        }
        head = next;
    }
    return head;
}

cons_t* eval_car_and_replace(cons_t* tree, cons_t* vars)
{
    assert(tree->type == LIST && tree->car != NULL);
    cons_t* cdr = tree->cdr;
    cons_t* ret = eval(tree->car, vars);
    *tree = *ret;
    tree->cdr = cdr;
    ret->car = NULL;
    free_tree(ret);
    return tree;
}

cons_t* eval(cons_t* tree, cons_t* vars)
{
    const char* op = NULL;
    int is_operator = 0;
   
    if(tree->type == LIST){
        eval_all(tree, vars); 
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
        int lhs, rhs, ret;
        int ret_type = NIL;

        cons_t* lhs_cell = NULL;
        cons_t* rhs_cell = NULL;

        assert(tree->cdr != NULL);
        assert(tree->cdr->cdr != NULL);

        lhs_cell = tree->cdr;
        rhs_cell = lhs_cell->cdr;

        if(rhs_cell->type == LIST){
            eval_car_and_replace(rhs_cell, vars);
        }
        if(lhs_cell->type == LIST){
            eval_car_and_replace(lhs_cell, vars);
        }

        while(lhs_cell->type == STR && vars != NULL){
            get_var_and_replace(vars, lhs_cell);
        }
        while(rhs_cell->type == STR && vars != NULL){
            get_var_and_replace(vars, rhs_cell);
        }
        
        assert(lhs_cell->type == INT);
        assert(rhs_cell->type == INT);

        lhs = lhs_cell->iValue;
        rhs = rhs_cell->iValue;

        switch(op[0]){
        case '+':
            ret = lhs + rhs;
            ret_type = INT;
            break;
        case '-':
            ret = lhs - rhs;
            ret_type = INT;
            break;
        case '/':
            ret = lhs / rhs;
            ret_type = INT;
            break;
        case '*':
            ret = lhs * rhs;
            ret_type = INT;
            break;
        case '<':
            ret = lhs < rhs;
            ret_type = ret ? TRUE : NIL;
            break;
        case '>':
            ret = lhs > rhs;
            ret_type = ret ? TRUE : NIL;
            break;
        case '=':
            ret = lhs == rhs;
            ret_type = ret ? TRUE : NIL;
            break;
        }
        free_tree(tree);
        return create_cons_cell(&ret, ret_type);
    }

    if(strcmp(op, "defun") == 0){
        cons_t* ret_cell = define_func(tree->cdr);
        tree->cdr = NULL;
        free_tree(tree);
        return ret_cell;
    }
    if(strcmp(op, "setq") == 0){
        cons_t* ret_cell = set_variable(vars, tree->cdr, tree->cdr->cdr);
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
            eval_car_and_replace(condition, vars);
        }

        if(on_true->type == STR && vars != NULL){
            get_var_and_replace(vars, on_true);
        }
        if(on_nil->type == STR && vars != NULL){
            get_var_and_replace(vars, on_nil);
        }

        if(condition->type == NIL){
            if(on_nil->type == LIST){
                ret_cell = eval(on_nil->car, vars);
                on_nil->car = NULL; 
            }else{
                ret_cell = copy_cell(on_nil);
            }
        }else{
            if(on_true->type == LIST){
                ret_cell = eval(on_true->car, vars);
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
            eval_all(tree->cdr, vars);
            ret_cell = apply(func, tree->cdr, vars);
            free_tree(tree);
            return ret_cell;
        }
    }

    if(vars){
        if(tree->cdr){
            free_tree(tree->cdr);
            tree->cdr = NULL;
        }
        return get_var_and_replace(vars, tree);
    }

    free_tree(tree);
    return create_cons_cell(NULL, NIL);
}

cons_t* apply(cons_t* function, cons_t* args, cons_t* upper_vars)
{
    assert(function->type == STR);
    
    cons_t* vars = NULL;
    cons_t* ret = NULL;
    
    cons_t* params = function->cdr->car;
    cons_t* proc = function->cdr->cdr->car;

    vars = create_cons_cell(params, LIST);
    vars->cdr = create_cons_cell(args, LIST);
    vars->cdr->cdr = upper_vars;
    
    ret = eval(copy_tree(proc), vars);
    
    vars->cdr->cdr = NULL;
    vars->cdr->car = NULL;
    vars->car = NULL;
   
    free_tree(vars);
    
    return ret;
}


