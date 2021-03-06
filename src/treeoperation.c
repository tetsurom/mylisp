#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "treeoperation.h"

cons_t* create_cons_cell(void* value, int type)
{
    cons_t* cell = NULL;

    cell = (cons_t*)malloc(sizeof(cons_t));

    cell->type = type;
    switch(type){
    case STR:
    {
        size_t len;
        assert(value != NULL);
        len = strlen((char*)value);
        if(len != 0){
            cell->svalue = (char*)malloc(sizeof(char) * (len + 1));
            strcpy(cell->svalue, (char*)value);
        }else{
            cell->type = NIL;
        }
        break;
    }
    case LIST:
    {
        assert(value != NULL);
        cell->car = (cons_t*)value;
        if(cell->car == NULL){
            cell->type = NIL;
        }
        break;
    }
    case INT:
    {
        assert(value != NULL);
        cell->iValue = *(int*)value;
        break;
    }
    case NIL:
    case TRUE:
    default:
        break;
    }
    cell->cdr = NULL;
    return cell;
}

void print_tree(cons_t* cell)
{
    switch(cell->type){
    case LIST:
        putchar('(');
        print_tree(cell->car);
        putchar(')');
        break;
    case STR:
        printf("%s", cell->svalue);
        break;
    case INT:
        printf("%d", cell->iValue);
        break;
    case NIL:
        printf("nil");
        break;
    case TRUE:
        printf("true");
        break;
    default:
        break;
    }
    if(cell->cdr != NULL){
        putchar(' ');
        print_tree(cell->cdr);
    }
}

void free_tree(cons_t* cell)
{
    if(cell == NULL){
        return;
    }
    
    if(cell->type == LIST && cell->car != NULL){
        free_tree(cell->car);
    }else if(cell->type == STR && cell->svalue != NULL){
        free(cell->svalue);
    }
    if(cell->cdr != NULL){
        free_tree(cell->cdr);
    }
    memset(cell, 0, sizeof(cons_t)); 
    free(cell);
}

cons_t* copy_tree(cons_t* tree_head)
{
    cons_t* copied_tree_head = NULL;
    assert(tree_head != NULL);
    copied_tree_head = copy_cell(tree_head);
    if(tree_head->cdr != NULL){
        copied_tree_head->cdr = copy_tree(tree_head->cdr);
    }
    return copied_tree_head;
}

cons_t* copy_cell(cons_t* cell)
{
    cons_t* copied_cell = NULL;
    assert(cell != NULL);
    switch(cell->type){
    case INT:
        copied_cell = create_cons_cell(&cell->iValue, INT);
        break;
    case STR:
        copied_cell = create_cons_cell(cell->svalue, STR);
        break;
    case LIST:
        copied_cell = create_cons_cell(copy_tree(cell->car), LIST);
        break;
    default:
        copied_cell = create_cons_cell(NULL, cell->type);
        break;
    }
    return copied_cell;
}
