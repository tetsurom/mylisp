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
#include "istack.h"
#include "lisp.h"
#include "compiler.h"

FILE* open_stream(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
    lisp_t* L = NULL;
    cons_t* tree = NULL;
    cons_t* head;

    L = lisp_open();
    {
        FILE* file = open_stream(argc, argv);
        tree = lisp_parseFromStream(L, file);
        if(file != stdin){
            fclose(file);
        }
    }
    for(head = tree; head; head = head->cdr){
        print_tree(head);
        putchar('\n');
        lisp_eval(L, head);   
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


