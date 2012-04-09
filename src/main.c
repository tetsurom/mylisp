#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

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
cons_t* copy_cell(cons_t* cell);
cons_t* copy_tree(cons_t* tree);
cons_t* eval(cons_t* tree, cons_t* vars);
cons_t* define_func(cons_t* definition);
cons_t* apply(cons_t* function, cons_t* args);
cons_t* get_var(cons_t* vars, cons_t* name);

cons_t* function;

int main(int argc, const char* argv[])
{
    char buf[1024];
    FILE* file = open_stream(argc, argv);
    cons_t* cell = get_tree(file, buf, sizeof(buf) / sizeof(char));
    function = NULL;
    print_tree(cell);
    putchar('\n');
    cell = eval(cell, NULL);
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
    if( (file = fopen(argv[1], "r")) ){
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
        cell->svalue[0] = '\0';
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

cons_t* eval(cons_t* tree, cons_t* vars)
{
    const char* op = NULL;
    int lhs, rhs, ret;
    int ret_type;
    
    cons_t* lhs_cell = NULL;
    cons_t* rhs_cell = NULL;
    cons_t* rhs_cell_cdr = NULL;
    cons_t* ret_cell = NULL;

    if(tree->type == LIST){
        cons_t* head = tree;
        while(head != NULL){
            cons_t* next = head->cdr;
            if(head->type == LIST){
                cons_t* ret = eval(head->car, vars);
                *head = *ret;
                head->cdr = next;
                free_tree(ret);
            }
            head = next;
        }
        return tree;
    }

    if(tree->type != STR){
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
            assert(condition->car != NULL);
            *condition = *eval(condition->car, vars);
            condition->cdr = on_true;
        }

        if(on_true->type == STR){
            if(vars != NULL){
                on_true->cdr = NULL;
                on_true = get_var(vars, on_true);
                condition->cdr = on_true;
                on_true->cdr = on_nil;
            }
        }
        if(on_nil->type == STR){
            if(vars != NULL){
                cons_t* cdr = on_nil->cdr;
                on_nil->cdr = NULL;
                on_nil = get_var(vars, on_nil);
                on_true->cdr = on_nil;
                on_nil->cdr = cdr;
            }
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
        cons_t* head = tree->cdr;
        while(head != NULL){
            cons_t* next = head->cdr;
            if(head->type == LIST){
                cons_t* ret = eval(head->car, vars);
                *head = *ret;
                head->cdr = next;
                free_tree(ret);
            }
            head = next;
        }
        ret_cell = apply(function, tree->cdr);
        free_tree(tree);
        return ret_cell;
    }

    lhs_cell = tree->cdr;
    rhs_cell = lhs_cell->cdr;
    rhs_cell_cdr = rhs_cell->cdr;

    if(rhs_cell->type == LIST){
        cons_t* ret = eval(rhs_cell->car, vars);
        *rhs_cell = *ret;
        rhs_cell->cdr = rhs_cell_cdr;
        free_tree(ret);
    }
    if(lhs_cell->type == LIST){
        cons_t* ret = eval(lhs_cell->car, vars);
        *lhs_cell = *ret;
        lhs_cell->cdr = rhs_cell;
        free_tree(ret);
    }

    assert(lhs_cell != NULL);
    if(lhs_cell->type == STR){
        if(vars != NULL){
            lhs_cell->cdr = NULL;
            lhs_cell = get_var(vars, lhs_cell);
            tree->cdr = lhs_cell;
            lhs_cell->cdr = rhs_cell;
        }
    }
    assert(rhs_cell != NULL);
    if(rhs_cell->type == STR){
        if(vars != NULL){
            rhs_cell->cdr = NULL;
            rhs_cell = get_var(vars, rhs_cell);
            lhs_cell->cdr = rhs_cell;
            rhs_cell->cdr = rhs_cell_cdr;
        }
    }

    
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
    }
    
    free_tree(tree);
    return create_cons_cell(&ret, ret_type);
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

    vars = create_cons_cell(function->cdr->car, LIST);
    vars->cdr = create_cons_cell(args, LIST);
    
    ret = eval(copy_tree(function->cdr->cdr->car), vars);
    
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
            free_tree(name);
            return ret;
        }
        names = names->cdr;
        values = values->cdr;
    }
    free_tree(name);
    return create_cons_cell(NULL, NIL);
}


