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
#include "lisp.h"

FILE* open_stream(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
    lisp_t* L = NULL;
    FILE* file = open_stream(argc, argv);
    cons_t* tree = NULL;
    cons_t* head;

    L = lisp_open();

    tree = lisp_parseFromStream(L, file);
    if(file != stdin){
        fclose(file);
    }
    //print_tree((cons_t*)stack_get(L->g_stack, -1));
    print_tree(tree);
    putchar('\n');
    for(head = tree; head; head = head->cdr){
        if(head->type == LIST){
            lisp_eval(L, head, 0);
            print_tree((cons_t*)stack_get(L->g_stack, -1));
            putchar('\n');
            stack_clear(L->g_stack);
        }
    }
    free_tree(tree);
    lisp_close(L);
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


