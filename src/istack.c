#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "istack.h"

static int istack_calcindex(istack_t* self, int index)
{
    int i;
    assert(self);
    i = index < 0 ? self->top + index + 1 : index;
    assert(i >= 0 && i <= self->top);
    return i;
}

istack_t* istack_create(size_t size)
{
    istack_t* self = (istack_t*)malloc(sizeof(istack_t));
    assert(size > 0);
    self->size = size;
    self->top = -1;
    self->data = malloc(sizeof(int) * size);
    return self;
}

int istack_get(istack_t* self, int index)
{
    return self->data[istack_calcindex(self, index)];
}

void istack_settop(istack_t* self, int top)
{
    assert(self);
    assert(top >= -1 && top < (int)self->size);
    self->top = top;
}

void istack_push(istack_t* self, const int value)
{
    assert(self);
    self->top += 1;
    assert(self->top < self->size);
    self->data[self->top] = value;
}

void istack_pop(istack_t* self)
{
    assert(self);
    if(self->top >= 0){
        --self->top;
    }
}

int istack_top(istack_t* self)
{
    return self->data[self->top];
}

void istack_remove(istack_t* self, int index)
{
    int i = istack_calcindex(self, index);
    int top = self->top;
    int len = top - i;
    int isize = sizeof(int);
    if(i < top){
        memcpy(self->data + i * isize, self->data + (i + 1) * isize, isize * len);
    }
    --self->top;
}

void istack_clear(istack_t* self)
{
    assert(self);
    self->top = -1;
}

void istack_destroy(istack_t* self)
{
    assert(self);
    free(self->data);
    self->data = NULL;
    free(self);
}

int istack_empty(istack_t* self)
{
    assert(self);
    return self->top > -1;
}

