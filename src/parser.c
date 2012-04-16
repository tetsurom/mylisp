#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "treeoperation.h"
#include "parser.h"
#include "stack.h"

int get_token(FILE* file, char buf[], int buf_size);
struct cons_t* get_tree(FILE* file, char buf[], int buf_size);

cons_t* lisp_parseFromStream(lisp_t* L, FILE* file)
{
    char buf[1024];
    cons_t* tree = get_tree(file, buf, sizeof(buf) / sizeof(char));
    //stack_push(L->g_stack, tree);
    return tree;
}

int get_token(FILE* file, char buf[], int buf_size)
{
    int len = 0;
    char ch = fgetc(file);
    while(ch != EOF && len < buf_size){
        switch(ch){
        case '(':
        case ')':
            if(len != 0){
                ungetc(ch, file);
            }else{
                buf[len++] = ch;
            }
            goto END_OF_TOKEN;
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            if(len != 0){
                goto END_OF_TOKEN;
            }else{
                break;
            }
        default:
            buf[len++] = ch;
            break;
        }
        ch = fgetc(file);
    }
END_OF_TOKEN:
    buf[len] = '\0';
    return len;
}

cons_t* get_tree(FILE* file, char buf[], int buf_size)
{
    cons_t* cell = NULL;
    if(0 != get_token(file, buf, buf_size)){
        printf("%s\n", buf);
        if(strcmp(buf, ")") == 0){
            return cell;
        }
        if(strcmp(buf, "(") == 0){
            cell = create_cons_cell((void*)get_tree(file, buf, buf_size), LIST);
        }else{
            int type;
            long sl;
            char* end_ptr;

            errno = 0;
            sl = strtol(buf, &end_ptr, 10);

            if((sl == LONG_MIN || sl == LONG_MAX) && errno != 0){
                type = STR;
            }else if(end_ptr == buf){
                type = STR;
            }else if('\0' != *end_ptr){
                type = STR;
            }else {
                type = INT;
            }
            if(type == STR){
                cell = create_cons_cell(buf, STR);
            }else{
                cell = create_cons_cell(&sl, INT);
            }
        }
        cell->cdr = get_tree(file, buf, buf_size);
    }
    return cell;
}

