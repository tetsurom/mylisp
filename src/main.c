#include <stdio.h>
#include <string.h>
#include <memory.h>

enum cell_type_e {INT, STR, LIST};

typedef struct cons_t{
    int type;
    union {
        struct cons_t* car;
        int iValue;
        const char* svalue;
    };
    struct cons_t* cdr;
} cons_t;


FILE* open_stream(int argc, const char* argv[]);
int get_token(FILE* file, char buf[], int buf_size);
cons_t* create_cons_cell(const char* sValue);
cons_t* get_tree(FILE* file);
void print_tree(cons_t* tree);
void free_tree(cons_t* cell);

int main(int argc, const char* argv[])
{
    FILE* file = open_stream(argc, argv);
    cons_t* cell = get_tree(file);
    print_tree(cell);
    printf("\n");
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

cons_t* create_cons_cell(const char* sValue)
{
    cons_t* cell = (cons_t*)malloc(sizeof(cons_t));
    size_t len = strlen(sValue);
    if(len != 0){
        cell->svalue = (char*)malloc(sizeof(char) * len);
        strcpy(cell->svalue, sValue);
        cell->type = STR;
    }
    cell->cdr = NULL;
    return cell;
}

cons_t* get_tree(FILE* file)
{
    char token[1024];
    cons_t* cell = NULL;
    if(0 != get_token(file, token, sizeof(token) / sizeof(char))){
        printf("%s\n", token);
        if(strcmp(token, ")") == 0){
            return cell;
        }
        if(strcmp(token, "(") == 0){
            cell = create_cons_cell("");
            cell->car = get_tree(file);
            cell->type = LIST;
        }else{
            cell = create_cons_cell(token);
        }
        cell->cdr = get_tree(file);
    }
    return cell;
}

void print_tree(cons_t* cell)
{
    if(cell->type == LIST){
        printf("(");
        print_tree(cell->car);
        printf(")");
    }else if(cell->type == STR){
        printf("%s", cell->svalue);
    }
    if(cell->cdr != NULL){
        printf(" ");
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

