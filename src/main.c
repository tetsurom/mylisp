#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

enum cell_type_e {NIL, TRUE, INT, STR, LIST};

typedef struct cons_t{
    int type;
    union {
        struct cons_t* car;
        int iValue;
        char* svalue;
    };
    struct cons_t* cdr;
} cons_t;


FILE* open_stream(int argc, const char* argv[]);
int get_token(FILE* file, char buf[], int buf_size);
cons_t* create_cons_cell(void* value, int type);
cons_t* get_tree(FILE* file, char buf[], int buf_size);
void print_tree(cons_t* tree);
void free_tree(cons_t* cell);
cons_t* eval(cons_t* tree);

int main(int argc, const char* argv[])
{
    char buf[1024];
    FILE* file = open_stream(argc, argv);
    cons_t* cell = get_tree(file, buf, sizeof(buf) / sizeof(char));
    print_tree(cell);
    putchar('\n');
    cell = eval(cell);
    print_tree(cell);
    putchar('\n');
    free_tree(cell);

    if(file != stdin) fclose(file);
    return 0;
}

FILE* open_stream(int argc, const char* argv[])
{
    FILE* file;
    if(argc < 2) return stdin;
    if(file = fopen(argv[1], "r")){
        return file;
    }
    return stdin;
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

cons_t* create_cons_cell(void* value, int type)
{
    cons_t* cell = (cons_t*)malloc(sizeof(cons_t));

    cell->type = type;
    switch(type){
    case STR:{
        size_t len = strlen((char*)value);
        if(len != 0){
            cell->svalue = (char*)malloc(sizeof(char) * len);
            strcpy(cell->svalue, (char*)value);
        }else{
            cell->type = NIL;
        }
        break;
    }case LIST:{
        cell->car = (cons_t*)value;
        if(cell->car == NULL){
            cell->type = NIL;
        }
        break;
    }case INT:{
        cell->iValue = *(int*)value;
        break;
    }case NIL:
    case TRUE:
    default:
        break;
    }
    cell->cdr = NULL;
    return cell;
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
    defaut:
        break;
    }
    if(cell->cdr != NULL){
        putchar(' ');
        print_tree(cell->cdr);
    }
}

void free_tree(cons_t* cell)
{
    if(cell->type == LIST){
        free_tree(cell->car);
    }else if(cell->type == STR){
        free(cell->svalue);
    }
    if(cell->cdr != NULL){
        free_tree(cell->cdr);
    }
    free(cell);
}

cons_t* eval(cons_t* tree)
{
    const char* op;
    int lhs, rhs, ret;
    cons_t* lhs_cell;
    cons_t* rhs_cell;

    if(tree->type == LIST){
        return eval(tree->car);
    }

    op = tree->svalue;

    lhs_cell = tree->cdr;
    rhs_cell = lhs_cell->cdr;

    if(rhs_cell->type == LIST){
        rhs_cell = eval(rhs_cell);
    }
    if(lhs_cell->type == LIST){
        lhs_cell = eval(lhs_cell);
    }
    
    lhs = lhs_cell->iValue;
    rhs = rhs_cell->iValue;

    switch(op[0]){
    case '+':
        ret = lhs + rhs;
        break;
    case '-':
        ret = lhs - rhs;
        break;
    case '/':
        ret = lhs / rhs;
        break;
    case '*':
        ret = lhs * rhs;
        break;
    }
    
    free_tree(tree);
    return create_cons_cell(&ret, INT);
}


