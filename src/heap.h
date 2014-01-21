#ifndef HEAP_H
#define HEAP_H

#include "vector.h"

typedef struct
{
  vector_t *vec;
  int (*cmp)(void *left, void *right);
} heap_t;

typedef size_t heap_node_t;

#define INVALID_HEAP_NODE -1

heap_t *heap_new(int (*cmp)(void *left, void *right));

void heap_free(heap_t **h, int free_items);

void heap_push(heap_t *h, void *item);

void *heap_pop(heap_t *h);

static inline int heap_empty(heap_t const *h)
{
  return vector_empty(h->vec);
}

static inline int heap_size(heap_t const *h)
{
  return vector_size(h->vec);
}

static inline int heap_node_isroot(heap_t const *h, heap_node_t n)
{
  return n == 0;
}

static inline void *heap_peek(heap_t const *h)
{
  return heap_empty(h) ? NULL : vector_get(h->vec, 0);
}

#define HEAP_NODE_LEFT(n) ( (n << 1) + 1 )
#define HEAP_NODE_RIGHT(n) ( (n << 1) + 2 )
#define HEAP_NODE_PARENT(n) ( (n & 1) ? (n - 1) >> 1 : (n - 2) >> 1 )

static inline heap_node_t heap_node_left(heap_t *h, heap_node_t n)
{
  size_t l = HEAP_NODE_LEFT(n);
  return l >= vector_size(h->vec) ? INVALID_HEAP_NODE : l;
}

static inline heap_node_t heap_node_right(heap_t *h, heap_node_t n)
{
  size_t r = HEAP_NODE_RIGHT(n);
  return r >= vector_size(h->vec) ? INVALID_HEAP_NODE : r;
}

static inline heap_node_t heap_node_parent(heap_t *h, heap_node_t n)
{
  return n ? HEAP_NODE_PARENT(n) : INVALID_HEAP_NODE;
}

static inline void heap_nodes_swap(heap_t *h, heap_node_t a, heap_node_t b)
{
  vector_swap(h->vec, a, b);
}

#endif
