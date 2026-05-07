#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

stack_t *stack_new(size_t capacity) {
  stack_t *stack = malloc(sizeof(stack_t));
  if (stack == NULL) {
    return NULL;
  }
  stack->count    = 0;
  stack->capacity = capacity;
  stack->data     = malloc(stack->capacity * sizeof(void *));
  if (stack->data == NULL) {
    free(stack);
    return NULL;
  }
  return stack;
}

void stack_push(stack_t *stack, void *obj) {
  if (stack->count == stack->capacity) {
    stack->capacity *= 2;
    void **new_data = realloc(stack->data, stack->capacity * sizeof(void *));
    if (new_data == NULL) {
      fprintf(stderr, "stack_push: realloc failed\n");
      exit(1);
    }
    stack->data = new_data;
  }
  stack->data[stack->count++] = obj;
}

void *stack_pop(stack_t *stack) {
  if (stack->count == 0) {
    return NULL;
  }
  return stack->data[--stack->count];
}

void stack_free(stack_t *stack) {
  if (stack == NULL) {
    return;
  }
  if (stack->data != NULL) {
    free(stack->data);
  }
  free(stack);
}

void stack_remove_nulls(stack_t *stack) {
  size_t new_count = 0;

  // Compact non-NULL pointers to the front
  for (size_t i = 0; i < stack->count; i++) {
    if (stack->data[i] != NULL) {
      stack->data[new_count++] = stack->data[i];
    }
  }

  // Zero out the vacated slots
  for (size_t i = new_count; i < stack->capacity; i++) {
    stack->data[i] = NULL;
  }

  stack->count = new_count;
}
