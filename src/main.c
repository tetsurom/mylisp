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
cons_t* apply(cons_t* function, cons_t* args, cons_t* upper_vars);
cons_t* get_var(cons_t* vars, cons_t* name);
cons_t* get_var_and_replace(cons_t* vars, cons_t* name);
cons_t* get_func(cons_t* functions, cons_t* name);
cons_t* set_variable(cons_t* vars, cons_t* name, cons_t* value);

cons_t* g_functions;
cons_t* g_variables;

int main(int argc, const char* argv[])
{
    FILE* file = open_stream(argc, argv);
    cons_t* tree = parse_from_stream(file);
    print_tree(tree);
    putchar('\n');

    g_variables = create_cons_cell(NULL, NIL);
    g_variables->cdr = create_cons_cell(NULL, NIL);

    tree = eval(tree, g_variables);
    print_tree(tree);
    putchar('\n');
    
    if(g_functions){
        free_tree(g_functions);
    }
    if(g_variables){
        free_tree(g_variables);
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

cons_t* define_func(cons_t* definition)
{
    cons_t* func_name = NULL;
    cons_t* old_func = get_func(g_functions, definition);
    if(old_func){
        free_tree(old_func->cdr);
        free(old_func->svalue);
        *old_func = *definition;
        func_name = copy_cell(definition);
        definition->cdr = NULL;
        definition->svalue = NULL;
        free_tree(definition);
    }else{
        cons_t* function_cell = create_cons_cell(definition, LIST);
        if(g_functions == NULL){
            g_functions = function_cell;
        }else{
            cons_t* head = g_functions;
            while(head->cdr != NULL){
                head = head->cdr;
            }
            head->cdr = function_cell;
        }
        func_name = copy_cell(definition);
    }
    return func_name;
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

cons_t* get_var(cons_t* vars, cons_t* name)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    cons_t* upper_vars = NULL;
    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);

    names = vars->car;
    values = vars->cdr->car;
    upper_vars = vars->cdr->cdr;

    while(values != NULL){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* ret = copy_cell(values);
            return ret;
        }
        names = names->cdr;
        values = values->cdr;
    }
    if(upper_vars){
        return get_var(upper_vars, name);
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

cons_t* get_func(cons_t* functions, cons_t* name)
{
    cons_t* func;
   
    if(functions == NULL){
        return NULL;
    }

    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);

    for(func = functions; func != NULL; func = func->cdr){
        assert(func->type == LIST && func->car->type == STR);
        if(strcmp(name->svalue, func->car->svalue) == 0){
            return func->car;
        }
    }
    return NULL; 
}

cons_t* set_variable(cons_t* vars, cons_t* name, cons_t* value)
{
    cons_t* names = NULL;
    cons_t* values = NULL;
    
    assert(vars != NULL);
    assert(name != NULL);
    assert(name->type == STR);
    assert(name->svalue != NULL);
    
    if(value->cdr){
        free_tree(value->cdr);
        value->cdr = NULL;
    }
    name->cdr = NULL;   
 
    if(vars->type == NIL){
        vars->type = LIST;
        vars->car = name;
        vars->cdr->type = LIST;
        vars->cdr->car = value;
        return copy_cell(value);
    }

    names = vars->car;
    values = vars->cdr->car;

    for(;;){
        assert(names->svalue != NULL);
        if(strcmp(name->svalue, names->svalue) == 0){
            cons_t* ret = copy_cell(value);
            cons_t* cdr = values->cdr;
            if(values->type == STR){
                free(values->svalue);
            }else if(values->type == LIST){
                free_tree(values->car);
            }
            *values = *value;
            values->cdr = cdr;
            value->svalue = NULL;
            free_tree(name);
            free_tree(value);
            return ret;
        }
        if(names->cdr){
            assert(values->cdr);
            names = names->cdr;
            values = values->cdr;
        }else{
            break;
        }
    }
    if(value->cdr){
        free_tree(value->cdr);
        value->cdr = NULL;
    }
    if(name->cdr){
        free_tree(name->cdr);
        name->cdr = NULL;
    }

    values->cdr = value;
    names->cdr = name;

    return copy_cell(value);
}

