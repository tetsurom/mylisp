#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "treeoperation.h"
#include "parser.h"
#include "variable.h"
#include "function.h"
#include "eval.h"
#include "stack.h"

FILE* open_stream(int argc, const char* argv[]);

stack_t* g_stack;

int main(int argc, const char* argv[])
{
    FILE* file = open_stream(argc, argv);
    cons_t* tree = parse_from_stream(file);
    print_tree(tree);
    putchar('\n');

    g_stack = stack_create(sizeof(cons_t), 1024);

    g_variables = create_cons_cell(NULL, NIL);
    g_variables->cdr = create_cons_cell(NULL, NIL);

    tree = eval(tree, g_variables, g_stack);
    print_tree((cons_t*)stack_get(g_stack, -1));
    putchar('\n');
    
    if(g_functions){
        free_tree(g_functions);
    }
    if(g_variables){
        free_tree(g_variables);
    }
    stack_destroy(g_stack);
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


