#ifndef VECTOR_H
#define VECTOR_H

#include <string.h>
#include <stdlib.h>
#include "config.h" // just to get inline to work on mscv

typedef struct
{
  void **data;
  size_t size;
  size_t capacity;
} vector_t;

vector_t *vector_new(size_t initial_size);

void vector_free(vector_t **vec, int free_data);

void vector_resize(vector_t *vec, size_t newsize);

void vector_insert(vector_t *vec, size_t i, void *val);

void *vector_remove(vector_t *vec, size_t i);

void vector_swap(vector_t *vec, size_t a, size_t b);

static inline size_t vector_size(vector_t const *vec)
{
  return vec->size;
}

static inline int vector_empty(vector_t const *vec)
{
  return vec->size == 0;
}

static inline void *vector_get(vector_t *vec, size_t i)
{
  return vec->data[i];
}

static inline void vector_set(vector_t *vec, size_t i, void *val)
{
  vec->data[i] = val;
}

static inline void *vector_begin(vector_t *vec)
{
  return vec->data[0];
}

static inline void *vector_end(vector_t *vec)
{
  return vec->data[vec->size];
}

static inline void vector_push_back(vector_t *vec, void *item)
{
  vector_insert(vec, vec->size, item);
}

static inline void vector_push_front(vector_t *vec, void *item)
{
  vector_insert(vec, 0, item);
}

static inline void *vector_pop_back(vector_t *vec)
{
  return vector_remove(vec, vec->size - 1);
}

static inline void *vector_pop_front(vector_t *vec)
{
  return vector_remove(vec, 0);
}

#endif
