#ifndef PARSER_H
#define PARSER_H

struct FILE;
struct cons_t;
struct lisp_t;

struct cons_t* lisp_parseFromStream(struct lisp_t* L, FILE* file);

#endif

