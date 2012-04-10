#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

struct cons_t;

struct cons_t* parse_from_stream(FILE* file);
FILE* open_stream(int argc, const char* argv[]);
int get_token(FILE* file, char buf[], int buf_size);
struct cons_t* get_tree(FILE* file, char buf[], int buf_size);

#endif

