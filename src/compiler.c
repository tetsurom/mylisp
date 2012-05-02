#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.h"
#include "treeoperation.h"
#include "function.h"
#include "variable.h"
#include "lisp.h"
#include "util.h"
#include "eval.h"

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
            if(!param){
                func->type = OP_ADD;
                int zero = 0;
                param = func->cdr = create_cons_cell(&zero, INT);
                ++state->pc;
                break;
            }
            if(!param->cdr){
                int zero = 0;
                param->cdr = create_cons_cell(&zero, INT);
            }
            if(!param->cdr->cdr){
                int zero = 0;
                param->cdr->cdr = create_cons_cell(&zero, INT);
            }
            cons_t* args = param->cdr->car;
            cons_t* proc = param->cdr->cdr;
            lisp_mn_t* funccode;
            bindParam(args, proc);
            // reserve function
            define_func(L, param->svalue, NULL, 0);
            funccode = lisp_compile(L, proc);
            define_func(L, param->svalue, funccode, tree_listsize(args));
            lisp_printcode(funccode);
            lisp_postprocess(funccode);
            break;
        }
        case SETQ:
        {
            cons_t* v_value = param->cdr;
            lisp_precompile(L, state, v_value);
            ++state->pc;
            break;
        }
        case IF:
        {
            cons_t* on_true = param->cdr;
            cons_t* on_nil = on_true->cdr;
            if(!on_nil){
                int zero = 0;
                on_nil = on_true->cdr = create_cons_cell(&zero, INT);
            }
            lisp_precompile(L, state, param);
            ++state->pc;
            lisp_precompile(L, state, on_true);
            ++state->pc;
            lisp_precompile(L, state, on_nil);
            break;
        }
        default:
        {
            int is_op = func->type != STR;
            if(!param){
                int zero = 0;
                param = func->cdr = create_cons_cell(&zero, INT);
            }
            lisp_precompile(L, state, param);
            for(param = param->cdr; param; param = param->cdr){
                if(param->type == INT && is_op){
                    ++state->pc;
                }else{
                    lisp_precompile(L, state, param); 
                    if(is_op){
                        ++state->pc;
                    }
                }
            }
            if(!is_op){
                ++state->pc;
            }
            break;
        }   
        } 
        break;
    }
    case PARAM:
    case TRUE:
    case NIL:
        ++state->pc;
        break;
    case STR:
        ++state->pc;
        break;
    case INT:
        ++state->pc;
        break;
    default:
        break;
    }
}

inline static int getOpecode(enum cell_type_e cell_type)
{
    return cell_type - OP_ADD + LC_ADD;
}

inline static int getCOpecode(enum cell_type_e cell_type)
{
    return cell_type - OP_ADD + LC_ADDC;
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
            compile(L, state, param->cdr);
            lisp_set_var(L, param->svalue, 0);
            mnemonic = state->bytecode + state->pc;
            mnemonic->opcode = LC_SETQ;
            mnemonic->poperand = lisp_get_var(L, param->svalue);
            ++state->pc;
            break;
        }
        case IF:
        {
            cons_t* on_true = param->cdr;
            cons_t* on_nil = on_true->cdr;
            int* nil_label = NULL;
            int* endif_label = NULL;
            lisp_mn_t* jump_endif_mn = NULL;

            compile(L, state, param);

            mnemonic = state->bytecode + state->pc;
            mnemonic->opcode = LC_IFNIL;
            nil_label = &mnemonic->ioperand;
            ++state->pc;

            compile(L, state, on_true);
           
            mnemonic = state->bytecode + state->pc; 
            mnemonic->opcode = LC_JUMP;
            endif_label = &mnemonic->ioperand;
            jump_endif_mn = mnemonic;
            ++state->pc;

            *nil_label = state->pc;
            
            compile(L, state, on_nil);

            *endif_label = state->pc;
            if((state->bytecode + *endif_label)->opcode == LC_RET){
                jump_endif_mn->opcode = LC_RET;
            }
            break;
        }
        default:
        {
            int is_op = func->type != STR;
            compile(L, state, param);
            if(!param->cdr && func->type == OP_SUB){
                state->bytecode[state->pc - 1].ioperand *= -1;
            }
            for(param = param->cdr; param; param = param->cdr){
                if(param->type == INT && is_op){
                    mnemonic = state->bytecode + state->pc;
                    mnemonic->opcode = getCOpecode(func->type);
                    mnemonic->ioperand = param->iValue;
                    ++state->pc;
                }else{
                    compile(L, state, param); 
                    mnemonic = state->bytecode + state->pc;
                    if(is_op){
                        mnemonic->opcode = getOpecode(func->type);
                        ++state->pc;
                    }
                }
            }
            mnemonic = state->bytecode + state->pc;
            if(!is_op){
                lisp_func_t* function = get_func(L, func->svalue); 
                if(!function){
                    mnemonic->opcode = LC_CALLS;
                    mnemonic->poperand = lisp_addsymbol(L, func->svalue);
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
    case STR:
    {
        int* var = lisp_get_var(L, tree->svalue);
        if(var){
            mnemonic->opcode = LC_LOADV;
            mnemonic->poperand = (void*)var;
        }else{
            mnemonic->opcode = LC_LOADVS;
            mnemonic->poperand = tree->svalue;
        }
        ++state->pc;
        break;
    }
    case INT:
        mnemonic->opcode = LC_PUSH;
        mnemonic->ioperand = tree->iValue;
        ++state->pc;
        break;
    case TRUE:
        mnemonic->opcode = LC_PUSH;
        mnemonic->ioperand = 1;
        ++state->pc;
        break;
    case NIL:
        mnemonic->opcode = LC_PUSH;
        mnemonic->ioperand = 0;
        ++state->pc;
        break;
    default:
        assert(0 && "compile error");
        break;
    }
}

void optimize(lisp_mn_t* bytecode){
    for(; bytecode->opcode != LC_END; ++bytecode){
        if(bytecode->opcode == LC_ADDC && bytecode->ioperand == 1){
            bytecode->opcode = LC_INC;
        }else if(bytecode->opcode == LC_SUBC && bytecode->ioperand == -1){
            bytecode->opcode = LC_INC;
        }else if(bytecode->opcode == LC_SUBC && bytecode->ioperand == 1){
            bytecode->opcode = LC_DEC;
        }else if(bytecode->opcode == LC_SUBC && bytecode->ioperand == 2){
            bytecode->opcode = LC_DEC2;
        }else if(bytecode->opcode == LC_PUSH && (bytecode + 1)->opcode == LC_RET ){
            bytecode->opcode = LC_RETV;
            (bytecode + 1)->opcode = LC_NOP;
        }
    }
}

lisp_mn_t* lisp_compile(lisp_t* L, cons_t* tree)
{
    lisp_compiler_state_t cstate;
    int codesize = 0;
    cstate.pc = 0;
    cstate.bytecode = NULL;
    lisp_precompile(L, &cstate, tree);
    if(tree->type != LIST || tree->car->type != DEFUN){
        lisp_mn_t* mnemonic;
        codesize = ++cstate.pc;
        //printf("ret\n");
        //printf("----------------\n");
        //printf("size: %d\n", codesize);

        cstate.bytecode = CALLOC(lisp_mn_t, codesize + 1);
        mnemonic = cstate.bytecode + codesize - 1;
        mnemonic->opcode = LC_RET;
        ++mnemonic;
        mnemonic->opcode = LC_END;
        cstate.pc = 0;
        compile(L, &cstate, tree);
        optimize(cstate.bytecode);
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
        case LC_ADDC:   return "ADDC";
        case LC_SUBC:   return "SUBC";
        case LC_MULC:   return "MULC";
        case LC_DIVC:   return "DIVC";
        case LC_EQC:    return "EQC";
        case LC_LSC:    return "LESSC";
        case LC_GRC:    return "GRTC";
        case LC_LSEQC:  return "LESSEQC";
        case LC_GREQC:  return "GRTEQC";
        case LC_NEQC:   return "NOTEQC";
        case LC_INC:    return "INC";
        case LC_DEC:    return "DEC";
        case LC_DEC2:    return "DEC2";
        case LC_RET:    return "RET";
        case LC_RETV:   return "RETV";
        case LC_NOP:    return "NOP";
        default: break;
    }
    return "";
}

void lisp_printcode(lisp_mn_t* code)
{
    int cnt = 0;
    if(!code) return;
    for(;code->opcode != LC_END; ++code){
        printf("0x%04X\t%s", cnt, getOpcodeStr(code));
        switch(code->opcode){
        case LC_PUSH:
        case LC_LOADP:
        case LC_RETV:
            printf("\t%d\n", code->ioperand);
            break;
        case LC_JUMP:
        case LC_IFNIL:
            printf("\t0x%04X\n", code->ioperand);
            break;
        case LC_LOADV:
        case LC_SETQ:
            printf("\t%p\n", code->poperand);
            break;
        case LC_CALL:
            printf("\t%p [%s]\n", code->poperand, ((lisp_list_t*)code->poperand)->name);
            //printf("\t%p\n", code->poperand);
            break;
        case LC_LOADVS:
        case LC_CALLS:
            printf("\t%s\n", (char*)code->poperand);
            break;
        default:
            if(code->opcode >= LC_ADDC && code->opcode <= LC_NEQC){
                printf("\t%d", code->ioperand);
            }
            putchar('\n');
            break;
        }
        ++cnt;
    }
}


void lisp_postprocess(struct lisp_mn_t* code)
{
    void** jumptable = getJumpTable();
    if(!code) return;
    for(;code->opcode != LC_END; ++code){
        code->popcode = jumptable[code->opcode];
    }
}

