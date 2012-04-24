#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "compiler.h"
#include "treeoperation.h"
#include "function.h"
#include "lisp.h"
#include "util.h"

typedef struct lisp_compiler_state_t{
    lisp_mn_t* bytecode;
    int pc;
} lisp_compiler_state_t;

static int getParamOrder(cons_t* params, cons_t* name)
{
    assert(name != NULL);
    assert(name->type == STR);
    int order = 0;

    while(params != NULL){
        if(strcmp(name->svalue, params->svalue) == 0){
            return order;
        }
        params = params->cdr;
        order++;
    }
    
    return -1;
}

static void bindParam(cons_t* params, cons_t* tree)
{
    if(tree == NULL){
        return;
    }
    
    if(tree->type == STR && tree->svalue != NULL){
        int order = getParamOrder(params, tree);
        if(order >= 0){
            free(tree->svalue);
            tree->type = PARAM;
            tree->iValue = order;
        }
    }
    if(tree->type == LIST){
        bindParam(params, tree->car);
    }{

}
    if(tree->cdr != NULL){
        bindParam(params, tree->cdr);
    }
}

static void lisp_precompile(lisp_t* L, lisp_compiler_state_t* state, cons_t* tree)
{
    switch(tree->type){
    case LIST:
    {
        cons_t* func = tree->car;
        cons_t* param = func->cdr;
        switch(func->type){
        case DEFUN:
        {
            cons_t* args = param->cdr->car;
            cons_t* proc = param->cdr->cdr;
            lisp_mn_t* funccode;
            bindParam(args, proc);
            printf("function %s (", param->svalue);
            print_tree(args);
            printf("):\n");
            // reserve function
            define_func(L, param->svalue, NULL, 0);
            funccode = lisp_compile(L, proc);
            define_func(L, param->svalue, funccode, tree_listsize(args));
            lisp_printcode(funccode);
            break;
        }
        case SETQ:
        {
            cons_t* v_name = param;
            cons_t* v_value = param->cdr;
            lisp_precompile(L, state, v_value);
            ++state->pc;
            printf("setq ");
            print_cell(v_name);
            putchar('\n');
            break;
        }
        case IF:
        {
            cons_t* on_true = param->cdr;
            cons_t* on_nil = on_true->cdr;
            lisp_precompile(L, state, param);
            ++state->pc;
            printf("ifnil N:");
            putchar('\n');
            lisp_precompile(L, state, on_true);
            ++state->pc;
            printf("goto ENDIF:\n");
            printf("N:\n");
            lisp_precompile(L, state, on_nil);
            printf("ENDIF:\n");
            break;
        }
        default:
        {
            int is_op = func->type != STR;
            lisp_precompile(L, state, param);
            for(param = param->cdr; param; param = param->cdr){
                lisp_precompile(L, state, param); 
                if(is_op){
                    ++state->pc;
                    print_cell(func);
                    putchar('\n');
                }
            }
            if(!is_op){
                ++state->pc;
                printf("call ");
                print_cell(func);
                putchar('\n');
            }
            break;
        }   
        } 
        break;
    }
    case PARAM:
        ++state->pc;
        printf("loadp ");
        print_cell(tree);
        putchar('\n');
        break;
    case STR:
        ++state->pc;
        printf("loadv ");
        print_cell(tree);
        putchar('\n');
        break;
    case INT:
        ++state->pc;
        printf("push ");
        print_cell(tree);
        putchar('\n');
        break;
    default:
        printf("ERROR\n");
        break;
    }
}

static int getOpecode(enum cell_type_e cell_type)
{
    return cell_type - OP_ADD + LC_ADD;
}

static char* strclone(const char* str)
{
    char* clone = CALLOC(char, strlen(str) + 1);
    strcpy(clone, str);
    return clone;
}

static void compile(lisp_t* L, lisp_compiler_state_t* state, cons_t* tree)
{
    int pc = state->pc;
    lisp_mn_t* mnemonic = state->bytecode + pc;
    switch(tree->type){
    case LIST:
    {
        cons_t* func = tree->car;
        cons_t* param = func->cdr;
        switch(func->type){
        case DEFUN:
        {
            break;
        }
        case SETQ:
        {
            cons_t* v_name = param;
            cons_t* v_value = param->cdr;
            break;
        }
        case IF:
        {
            cons_t* on_true = param->cdr;
            cons_t* on_nil = on_true->cdr;
            int* nil_label = NULL;
            int* endif_label = NULL;
            compile(L, state, param);

            mnemonic = state->bytecode + state->pc;
            mnemonic->opcode = LC_IFNIL;
            nil_label = &mnemonic->ioperand;
            ++state->pc;

            compile(L, state, on_true);
           
            mnemonic = state->bytecode + state->pc; 
            mnemonic->opcode = LC_JUMP;
            endif_label = &mnemonic->ioperand;
            ++state->pc;

            *nil_label = state->pc;
            
            compile(L, state, on_nil);

            *endif_label = state->pc;
            
            printf("ENDIF:\n");
            break;
        }
        default:
        {
            int is_op = func->type != STR;
            compile(L, state, param);
            mnemonic = state->bytecode + state->pc;
            for(param = param->cdr; param; param = param->cdr){
                compile(L, state, param); 
                mnemonic = state->bytecode + state->pc;
                if(is_op){
                    mnemonic->opcode = getOpecode(func->type);
                    ++state->pc;
                }
            }
            mnemonic = state->bytecode + state->pc;
            if(!is_op){
                lisp_func_t* function = get_func(L, func->svalue); 
                if(!function){
                    mnemonic->opcode = LC_CALLS;
                    mnemonic->poperand = strclone(func->svalue);
                }else{
                    mnemonic->opcode = LC_CALL;
                    mnemonic->poperand = function;
                }
                ++state->pc;
            }
            break;
        }   
        } 
        break;
    }
    case PARAM:
        mnemonic->opcode = LC_LOADP;
        mnemonic->ioperand = tree->iValue;
        ++state->pc;
        break;
    case VAR:
        mnemonic->opcode = LC_LOADV;
        mnemonic->poperand = tree->pvalue;
        ++state->pc;
        break;
    case STR:
        mnemonic->opcode = LC_LOADVS;
        mnemonic->poperand = strclone(tree->svalue);
        ++state->pc;
        break;
    case INT:
        mnemonic->opcode = LC_PUSH;
        mnemonic->ioperand = tree->iValue;
        ++state->pc;
        break;
    default:
        assert(0 && "compile error");
        break;
    }
}

lisp_mn_t* lisp_compile(lisp_t* L, cons_t* tree)
{
    lisp_compiler_state_t cstate;
    cstate.pc = 0;
    cstate.bytecode = NULL;
    lisp_precompile(L, &cstate, tree);
    if(tree->car->type != DEFUN){
        ++cstate.pc;
        printf("ret\n");
        printf("----------------\n");
        printf("size: %d\n", cstate.pc);

        cstate.bytecode = CALLOC(lisp_mn_t, cstate.pc);
        cstate.pc = 0;
        compile(L, &cstate, tree);

        lisp_mn_t* mnemonic = cstate.bytecode + cstate.pc;
        mnemonic->opcode = LC_RET;
        ++cstate.pc;
    }
    return cstate.bytecode;
}

static const char* getOpcodeStr(lisp_mn_t* code)
{
    switch(code->opcode){
        case LC_PUSH:   return "PUSH";
        case LC_LOADP:  return "LDP";
        case LC_LOADV:  return "LDV";
        case LC_LOADVS: return "LDVS";
        case LC_CALL:   return "CALL";
        case LC_CALLS:  return "CALLS";
        case LC_SETQ:   return "SETQ";
        case LC_IFNIL:  return "IFNIL";
        case LC_JUMP:   return "JUMP";
        case LC_ADD:    return "ADD";
        case LC_SUB:    return "SUB";
        case LC_MUL:    return "MUL";
        case LC_DIV:    return "DIV";
        case LC_EQ:     return "EQ";
        case LC_LS:     return "LESS";
        case LC_GR:     return "GRT";
        case LC_LSEQ:   return "LESSEQ";
        case LC_GREQ:   return "GRTEQ";
        case LC_NEQ:    return "NOTEQ";
        case LC_RET:    return "RET";
        default: break;
    }
    return "";
}

void lisp_printcode(lisp_mn_t* code)
{
    int cnt = 0;
    if(!code) return;
    --code;
    do{
        ++code;
        printf("0x%04X\t%s", cnt, getOpcodeStr(code));
        switch(code->opcode){
        case LC_PUSH:
        case LC_LOADP:
            printf("\t%d\n", code->ioperand);
            break;
        case LC_JUMP:
        case LC_IFNIL:
            printf("\t0x%04X\n", code->ioperand);
            break;
        case LC_LOADV:
        case LC_CALL:
            printf("\t%p [%s]\n", code->poperand, ((lisp_list_t*)code->poperand)->name);
            //printf("\t%p\n", code->poperand);
            break;
        case LC_LOADVS:
        case LC_CALLS:
        case LC_SETQ:
            printf("\t%s\n", (char*)code->poperand);
            break;
        default:
            putchar('\n');
            break;
        }
        ++cnt;
    }while(code->opcode != LC_RET);
}

