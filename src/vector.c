#include "vector.h"
#include "utils.h"

vector_t *vector_new(size_t initial_size)
{
  vector_t *v = (vector_t *)lothar_malloc(sizeof(vector_t));

  v->size = initial_size;
  v->capacity = initial_size > 1 ? initial_size : 1;

  v->data = (void **)lothar_malloc(v->capacity * sizeof(void *));
  return v;
}

void vector_free(vector_t **vec, int free_data)
{
  if(free_data)
  {
    size_t i;
    for(i = 0; i < (*vec)->size; ++i)
      free((*vec)->data[i]);
  }

  free((*vec)->data);
  free(*vec);
  *vec = NULL;
}

void vector_resize(vector_t *vec, size_t newsize)
{
  if(newsize > vec->capacity)
  {
    while(newsize > vec->capacity)
      vec->capacity <<= 1;

    vec->data = (void **)lothar_realloc(vec->data, vec->capacity * sizeof(void *));
    vec->size = newsize;
  }
  else
  {
    vec->size = newsize;

    if(vec->capacity > 4 && vec->size < vec->capacity >> 1)
    {
      while(vec->capacity > 4 && vec->size < vec->capacity >> 1)
	vec->capacity >>= 1;

      vec->data = (void **)lothar_realloc(vec->data, vec->capacity * sizeof(void *));
    }
  }
}

void vector_insert(vector_t *vec, size_t i, void *item)
{
  size_t _i;

  vector_resize(vec, vec->size + 1);

  for(_i = vec->size; _i > i; --_i)
    vec->data[_i] = vec->data[_i - 1];
  
  vec->data[i] = item;
}

void *vector_remove(vector_t *vec, size_t i)
{
  void *ret = vec->data[i];

  for(; i < vec->size; ++i)
    vec->data[i] = vec->data[i + 1];

  vector_resize(vec, vec->size - 1);

  return ret;
}

void vector_swap(vector_t *vec, size_t a, size_t b)
{
  if(a != b)
  {
    void *tmp = vector_get(vec, a);
    vector_set(vec, a, vector_get(vec, b));
    vector_set(vec, b, tmp);
  }
}
