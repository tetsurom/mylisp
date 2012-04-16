#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stack.h"

int stack_calcindex(stack_t* self, int index)
{
    int i;
    assert(self);
    i = index < 0 ? self->top + index + 1 : index;
    assert(i >= 0 && i <= self->top);
    return i;
}

stack_t* stack_create(size_t item_size, size_t size)
{
    stack_t* self = (stack_t*)malloc(sizeof(stack_t));
    assert(size > 0 && item_size > 0);
    self->item_size = item_size;
    self->size = size;
    self->top = -1;
    self->data = malloc(item_size * size);
    return self;
}

void* stack_get(stack_t* self, int index)
{
    return self->data + self->item_size * stack_calcindex(self, index);
}

void stack_settop(stack_t* self, int top)
{
    assert(self);
    assert(top >= -1 && top < (int)self->size);
    self->top = top;
}

void stack_push(stack_t* self, const void* value)
{
    assert(self);
    self->top += 1;
    assert(self->top < self->size);
    memcpy(self->data + self->top * self->item_size, value, self->item_size);
}

void stack_pop(stack_t* self)
{
    assert(self);
    if(self->top >= 0){
        --self->top;
    }
}
void* stack_top(stack_t* self)
{
    return stack_get(self, -1);
}

void stack_remove(stack_t* self, int index)
{
    int i = stack_calcindex(self, index);
    int top = self->top;
    int len = top - i;
    int isize = self->item_size;
    if(i < top){
        memcpy(self->data + i * isize, self->data + (i + 1) * isize, isize * len);
    }
    stack_pop(self);
}

void stack_clear(stack_t* self)
{
    assert(self);
    self->top = -1;
}

void stack_destroy(stack_t* self)
{
    assert(self);
    free(self->data);
    free(self);
}

int stack_empty(stack_t* self)
{
    assert(self);
    return self->top > -1;
}

