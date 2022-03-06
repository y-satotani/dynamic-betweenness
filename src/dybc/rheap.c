#include "rheap.h"
#include <limits.h>
#include <igraph/igraph.h>

void rheap_print(rheap_t* heap) {
  printf("min_key=%d, min_bucket=%d, min_pos=%d\n",
         heap->current_min_key,
         heap->current_min_bucket,
         heap->current_min_pos);
  for(int bi = 0; bi < 33; bi++) {
    printf("[%d]->", bi);
    igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)[bi];
    for(int pos = 0; pos < igraph_vector_ptr_size(bucket); pos++) {
      printf("(%d, %d), ", ((rheap_elem_t*)VECTOR(*bucket)[pos])->key, ((rheap_elem_t*)VECTOR(*bucket)[pos])->value);
    }
    printf("\n");
  }
}

#define _BSR(x) ((x) == 0 ? -1 : (31 - __builtin_clz(x)))
inline int _bsr(int x) {
  return x == 0 ? -1 : (31 - __builtin_clz(x));
}

void rheap_init(rheap_t* heap,
                igraph_integer_t capacity,
                igraph_integer_t last_deleted) {
  igraph_vector_ptr_init(&heap->buckets, 33);
  for(int i = 0; i < igraph_vector_ptr_size(&heap->buckets); i++) {
    igraph_vector_ptr_t* bucket
      = (igraph_vector_ptr_t*) malloc(sizeof(igraph_vector_ptr_t));
    igraph_vector_ptr_init(bucket, 0);
    VECTOR(heap->buckets)[i] = bucket;
  }
  igraph_vector_int_init(&heap->value_map, capacity);
  igraph_vector_bool_init(&heap->value_emap, capacity);
  heap->size = 0;
  heap->last_deleted_key = last_deleted;
  heap->current_min_key = INT_MAX;
  heap->current_min_bucket = 0;
  heap->current_min_pos = 0;
}

void rheap_destroy(rheap_t* heap) {
  for(int i = 0; i < igraph_vector_ptr_size(&heap->buckets); i++) {
    igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)[i];
    for(int j = 0; j < igraph_vector_ptr_size(bucket); j++) {
      free(VECTOR(*bucket)[j]);
    }
    if(bucket) {
      igraph_vector_ptr_destroy(bucket);
      free(bucket);
    }
  }
  igraph_vector_ptr_destroy(&heap->buckets);
  igraph_vector_int_destroy(&heap->value_map);
  igraph_vector_bool_destroy(&heap->value_emap);
}

void rheap_clear(rheap_t* heap, igraph_integer_t last_deleted) {
  for(int i = 0; i < igraph_vector_ptr_size(&heap->buckets); i++) {
    igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)[i];
    for(int j = 0; j < igraph_vector_ptr_size(bucket); j++) {
      free(VECTOR(*bucket)[j]);
    }
    igraph_vector_ptr_clear(bucket);
  }
  igraph_vector_int_fill(&heap->value_map, 0);
  igraph_vector_bool_fill(&heap->value_emap, 0);
  heap->size = 0;
  heap->last_deleted_key = last_deleted;
  heap->current_min_key = INT_MAX;
  heap->current_min_bucket = 0;
  heap->current_min_pos = 0;
}

igraph_bool_t rheap_empty(rheap_t* heap) {
  return rheap_size(heap) == 0;
}

igraph_integer_t rheap_size(rheap_t* heap) {
  return heap->size;
}

void rheap_push_with_index(rheap_t* heap,
                           igraph_integer_t elem,
                           igraph_integer_t index) {
  int bi = _BSR(index ^ heap->last_deleted_key) + 1;
  igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)[bi];
  rheap_elem_t* e = (rheap_elem_t*) malloc(sizeof(rheap_elem_t));
  e->key = index;
  e->value = elem;
  igraph_vector_ptr_push_back(bucket, e);
  if(heap->current_min_key > index) {
    heap->current_min_key = index;
    heap->current_min_bucket = bi;
    heap->current_min_pos = igraph_vector_ptr_size(bucket) - 1;
  }
  heap->size++;
  VECTOR(heap->value_map)[elem] = index;
  VECTOR(heap->value_emap)[elem] = 1;
}

igraph_integer_t rheap_min_index(rheap_t* heap) {
  igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)
    [heap->current_min_bucket];
  return ((rheap_elem_t*)VECTOR(*bucket)[heap->current_min_pos])->key;
}

igraph_integer_t rheap_delete_min(rheap_t* heap) {
  //heap->last_deleted_key = heap->current_min_key;
  rheap_elem_t* del_elem
    = VECTOR(*(igraph_vector_ptr_t*)VECTOR(heap->buckets)[heap->current_min_bucket])[heap->current_min_pos];
  igraph_integer_t last_deleted_value = del_elem->value;
  heap->last_deleted_key = del_elem->key;
  igraph_vector_ptr_remove
    (VECTOR(heap->buckets)[heap->current_min_bucket], heap->current_min_pos);
  free(del_elem);

  igraph_integer_t min_key = INT_MAX;
  igraph_integer_t min_bucket = 0;
  igraph_integer_t min_pos = 0;
  int first_bucket = 0;

  if(heap->current_min_bucket > 0) {
    igraph_vector_ptr_t* bucket
      = VECTOR(heap->buckets)[heap->current_min_bucket];
    for(int pos = 0; pos < igraph_vector_ptr_size(bucket); pos++) {
      rheap_elem_t* e = VECTOR(*bucket)[pos];
      int bi = _BSR(e->key ^ heap->last_deleted_key) + 1;
      igraph_vector_ptr_t* insert_bucket = VECTOR(heap->buckets)[bi];
      igraph_vector_ptr_push_back(insert_bucket, e);

      if(min_key > e->key) {
        min_key = e->key;
        min_bucket = bi;
        min_pos = igraph_vector_ptr_size(insert_bucket) - 1;
      }
    }
    igraph_vector_ptr_clear(bucket);
    first_bucket = min_bucket + 1;
  }

  heap->current_min_key = min_key;
  heap->current_min_bucket = min_bucket;
  heap->current_min_pos = min_pos;

  heap->size--;
  if(heap->size > 0 && heap->current_min_key == INT_MAX) {
    heap->current_min_bucket = first_bucket;
    while(heap->current_min_bucket < igraph_vector_ptr_size(&heap->buckets)
          && igraph_vector_ptr_size
          (VECTOR(heap->buckets)[heap->current_min_bucket]) == 0)
      heap->current_min_bucket++;

    if(heap->current_min_bucket < igraph_vector_ptr_size(&heap->buckets)) {
      igraph_vector_ptr_t* bucket
        = VECTOR(heap->buckets)[heap->current_min_bucket];
      heap->current_min_pos = 0;
      if(heap->current_min_bucket >= 0) {
        for(int pos = 0; pos < igraph_vector_ptr_size(bucket); pos++) {
          rheap_elem_t* e = VECTOR(*bucket)[pos];
          if(heap->current_min_key > e->key) {
            heap->current_min_key = e->key;
            heap->current_min_pos = pos;
          }
        }
      }
    }
  }

  VECTOR(heap->value_map)[last_deleted_value] = 0;
  VECTOR(heap->value_emap)[last_deleted_value] = 0;
  return last_deleted_value;
}



igraph_bool_t rheap_has_elem(rheap_t* heap, igraph_integer_t elem) {
  return VECTOR(heap->value_emap)[elem];
}

igraph_integer_t rheap_get(rheap_t* heap, igraph_integer_t elem) {
  return VECTOR(heap->value_map)[elem];
}

igraph_integer_t rheap_modify(rheap_t* heap,
                              igraph_integer_t elem,
                              igraph_integer_t index) {

  // find the bucket and position of elem
  igraph_integer_t prev_key = VECTOR(heap->value_map)[elem];
  int prev_bi = _BSR(prev_key ^ heap->last_deleted_key) + 1;
  igraph_vector_ptr_t* prev_bucket = VECTOR(heap->buckets)[prev_bi];
  int prev_pos = 0;
  rheap_elem_t* e = NULL;
  for(prev_pos = 0; prev_pos < igraph_vector_ptr_size(prev_bucket); prev_pos++) {
    e = VECTOR(*prev_bucket)[prev_pos];
    if(e->value == elem)
      break;
  }

  // remove and insert into bucket
  int bi = _BSR(index ^ heap->last_deleted_key) + 1;
  int pos;
  e->key = index;
  if(prev_bi != bi) {
    igraph_vector_ptr_t* bucket = VECTOR(heap->buckets)[bi];
    igraph_vector_ptr_remove(prev_bucket, prev_pos);
    igraph_vector_ptr_push_back(bucket, e);
    pos = igraph_vector_ptr_size(bucket) - 1;
  } else {
    pos = prev_pos;
  }

  // find minimum bucket and position
  if(index < heap->current_min_key) {
    heap->current_min_key = index;
    heap->current_min_bucket = bi;
    heap->current_min_pos = pos;
  } else if(heap->current_min_bucket == prev_bi &&
            heap->current_min_pos == prev_pos) {
    // the element with minimum key was modified
    heap->current_min_key = INT_MAX;
    while(igraph_vector_ptr_size
          (VECTOR(heap->buckets)[heap->current_min_bucket]) == 0)
      heap->current_min_bucket++;
    igraph_vector_ptr_t* bucket
      = VECTOR(heap->buckets)[heap->current_min_bucket];
    heap->current_min_pos = 0;
    if(heap->current_min_bucket >= 0) {
      for(int pos = 0; pos < igraph_vector_ptr_size(bucket); pos++) {
        rheap_elem_t* f = VECTOR(*bucket)[pos];
        if(heap->current_min_key > f->key) {
          heap->current_min_key = f->key;
          heap->current_min_pos = pos;
        }
      }
    }

  }

  VECTOR(heap->value_map)[elem] = index;

  return prev_key;
}
