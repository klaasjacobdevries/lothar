#include "heap.h"
#include "utils.h"

heap_t *heap_new(int (*cmp)(void *a, void *b))
{
  heap_t *h = (heap_t *)lothar_malloc(sizeof(heap_t));

  h->vec = vector_new(0);
  h->cmp = cmp;

  return h;
}

void heap_free(heap_t **heap, int free_items)
{
  vector_free(&(*heap)->vec, free_items);

  free(*heap);
  *heap = NULL;
}

static void up_heap(heap_t *h, heap_node_t node)
{
  heap_node_t parent;

  while((parent = heap_node_parent(h, node)) != INVALID_HEAP_NODE && h->cmp(vector_get(h->vec, node), vector_get(h->vec, parent)) > 0)
  {
    heap_nodes_swap(h, node, parent);
    up_heap(h, parent);

    node = parent;
  }
}

static void down_heap(heap_t *h, heap_node_t node)
{
  while(node != INVALID_HEAP_NODE)
  {
    heap_node_t left  = heap_node_left(h, node);
    heap_node_t right = heap_node_right(h, node);
    heap_node_t child = INVALID_HEAP_NODE;

    if(right == INVALID_HEAP_NODE && left == INVALID_HEAP_NODE)
      return;
    else if(right == INVALID_HEAP_NODE)
    {
      if(h->cmp(vector_get(h->vec, node), vector_get(h->vec, left)) < 0)
	child = left;
    }
    else if(left == INVALID_HEAP_NODE)
    {
      if(h->cmp(vector_get(h->vec, node), vector_get(h->vec, right)) < 0)
	child = right;
    }
    else
    {
      int cmp = h->cmp(vector_get(h->vec, left), vector_get(h->vec, right));
      if(cmp >= 0 && h->cmp(vector_get(h->vec, node), vector_get(h->vec, left)) < 0)
	child = left;
      else if(cmp < 0 && h->cmp(vector_get(h->vec, node), vector_get(h->vec, right)) < 0)
	child = right;
    }
    
    if(child != INVALID_HEAP_NODE)
      heap_nodes_swap(h, node, child);

    node = child;
  }
}

void heap_push(heap_t *h, void *item)
{
  vector_push_back(h->vec, item);

  if(h->cmp)
    up_heap(h, vector_size(h->vec) - 1);
}

void *heap_pop(heap_t *h)
{
  void *item;

  if(heap_empty(h))
    return NULL;

  vector_swap(h->vec, 0, vector_size(h->vec) - 1);
  item = vector_pop_back(h->vec);

  if(h->cmp)
    down_heap(h, 0);

  return item;
}
