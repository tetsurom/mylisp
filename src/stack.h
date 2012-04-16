#ifndef STACK_H
#define STACK_H

typedef struct stack_t{
    size_t item_size;
    size_t size;
    int top;
    void* data;
} stack_t;

stack_t* stack_create(size_t item_size, size_t size);
void* stack_get(stack_t* self, int index);
void stack_settop(stack_t* self, int top);
void stack_push(stack_t* self, const void* value);
void stack_pop(stack_t* self);
void* stack_top(stack_t* self);
void stack_remove(stack_t* self, int index);
void stack_clear(stack_t* self);
void stack_destroy(stack_t* self);
int stack_empty(stack_t* self);

#endif

