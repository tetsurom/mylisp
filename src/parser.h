#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef struct cons_t cons_t;
typedef struct lisp_t lisp_t;

cons_t* lisp_parseFromStream(lisp_t* L, FILE* file);

#endif

