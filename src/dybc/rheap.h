
#ifndef _RHEAP_H_
#define _RHEAP_H_

#include <igraph/igraph.h>

typedef struct rheap_elem_t {
  igraph_integer_t key;
  igraph_integer_t value;
} rheap_elem_t;

typedef struct rheap_t {
  igraph_vector_ptr_t buckets;
  igraph_vector_int_t value_map;
  igraph_vector_bool_t value_emap;
  igraph_integer_t size;
  igraph_integer_t last_deleted_key;
  igraph_integer_t current_min_key;
  igraph_integer_t current_min_bucket;
  igraph_integer_t current_min_pos;
} rheap_t;

void rheap_init(rheap_t* heap,
                igraph_integer_t capacity,
                igraph_integer_t last_deleted);
void rheap_destroy(rheap_t* heap);
void rheap_clear(rheap_t* heap, igraph_integer_t last_deleted);
igraph_bool_t rheap_empty(rheap_t* heap);
igraph_integer_t rheap_size(rheap_t* heap);
void rheap_push_with_index(rheap_t* heap,
                           igraph_integer_t elem,
                           igraph_integer_t index);
igraph_integer_t rheap_min_index(rheap_t* heap);
igraph_integer_t rheap_delete_min(rheap_t* heap);
igraph_bool_t rheap_has_elem(rheap_t* heap, igraph_integer_t elem);
igraph_integer_t rheap_get(rheap_t* heap, igraph_integer_t elem);
igraph_integer_t rheap_modify(rheap_t* heap,
                              igraph_integer_t elem,
                              igraph_integer_t index);

void rheap_print(rheap_t* heap);

#endif // _RHEAP_H_
