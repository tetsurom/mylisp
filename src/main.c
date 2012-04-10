#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "treeoperation.h"
#include "parser.h"

FILE* open_stream(int argc, const char* argv[]);

cons_t* eval_all(cons_t* head, cons_t* vars);
cons_t* eval_car_and_replace(cons_t* tree, cons_t* vars);
cons_t* eval(cons_t* tree, cons_t* vars);
cons_t* define_func(cons_t* definition);
cons_t* apply(cons_t* function, cons_t* args);
cons_t* get_var(cons_t* vars, cons_t* name);
cons_t* get_var_and_replace(cons_t* vars, cons_t* name);

cons_t* function;

int main(int argc, const char* argv[])
{
    FILE* file = open_stream(argc, argv);
    cons_t* tree = parse_from_stream(file);
    function = NULL;
    print_tree(tree);
    putchar('\n');

    tree = eval(tree, NULL);
    print_tree(tree);
    putchar('\n');
    
    if(function != NULL){
        free_tree(function);
    }
    free_tree(tree);

    return 0;
}

FILE* open_stream(int argc, const char* argv[])
{
    FILE* file;
    if(argc < 2) return stdin;
    if( (file = fopen(argv[1], "r")) ){
        return file;
    }
    return stdin;
}


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
    int lhs, rhs, ret;
    int ret_type = NIL;
    int is_operator = 0;

    cons_t* lhs_cell = NULL;
    cons_t* rhs_cell = NULL;
    cons_t* ret_cell = NULL;

    if(tree->type == LIST){
        eval_all(tree, vars); 
        return tree;
    }else if(tree->type != STR){
        return tree;
    }

    op = tree->svalue;

    if(strcmp(op, "defun") == 0){
        ret_cell = define_func(tree->cdr);
        tree->cdr = NULL;
        free_tree(tree);
        return ret_cell;
    }

    if(strcmp(op, "if") == 0){
        cons_t* condition = NULL;
        cons_t* on_true = NULL;
        cons_t* on_nil = NULL;

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

    if(function != NULL && strcmp(function->svalue, op) == 0){
        eval_all(tree->cdr, vars);
        ret_cell = apply(function, tree->cdr);
        free_tree(tree);
        return ret_cell;
    }

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

        if(lhs_cell->type == STR && vars != NULL){
            get_var_and_replace(vars, lhs_cell);
        }
        if(rhs_cell->type == STR && vars != NULL){
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

cons_t* define_func(cons_t* definition)
{
    cons_t* func_name = NULL;
    function = copy_tree(definition);
    func_name = create_cons_cell(definition->svalue, STR);
    free_tree(definition);
    return func_name;
}

cons_t* apply(cons_t* function, cons_t* args)
{
    cons_t* vars = NULL;
    cons_t* ret = NULL;
    
    cons_t* params = function->cdr->car;
    cons_t* proc = function->cdr->cdr->car;

    vars = create_cons_cell(params, LIST);
    vars->cdr = create_cons_cell(args, LIST);
    
    ret = eval(copy_tree(proc), vars);
    
    vars->cdr->car = NULL;
    vars->car = NULL;
    
    free_tree(vars);
    
    return ret;
}

cons_t* get_var(cons_t* vars, cons_t* name)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    
    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);

    names = vars->car;
    values = vars->cdr->car;

    while(values != NULL){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* ret = copy_cell(values);
            return ret;
        }
        names = names->cdr;
        values = values->cdr;
    }
    return create_cons_cell(NULL, NIL);
}

cons_t* get_var_and_replace(cons_t* vars, cons_t* name)
{
    assert(name->type == STR);
    cons_t* cdr = name->cdr;
    cons_t* ret = get_var(vars, name);
    free(name->svalue);
    *name = *ret;
    name->cdr = cdr;
    ret->car = NULL;
    free_tree(ret);
    return name;
}


