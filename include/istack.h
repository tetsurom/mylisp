#ifndef ISTACK_H
#define ISTACK_H

typedef struct istack_t{
    size_t size;
    int top;
    int* data;
} istack_t;

istack_t* istack_create(size_t size);
int istack_get(istack_t* self, int index);
void istack_settop(istack_t* self, int top);
void istack_push(istack_t* self, int value);
void istack_pop(istack_t* self);
int istack_top(istack_t* self);
void istack_remove(istack_t* self, int index);
void istack_clear(istack_t* self);
void istack_destroy(istack_t* self);
int istack_empty(istack_t* self);

#endif

