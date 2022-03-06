
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc/rheap.h"

/*
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
*/

int main(int argc, char* argv[]) {
  rheap_t heap;
  rheap_init(&heap, 100, 0);
  printf("size of heap is %d (heap is %sempty)\n",
         rheap_size(&heap),
         rheap_empty(&heap) ? "" : "not ");

  igraph_integer_t key[] = {3, 3, 5, 5, 6, 100, 1000};
  for(int v = 0; v < 7; v++) {
    igraph_integer_t k = key[v];
    printf("push %d with key=%d\n", v, k);
    rheap_push_with_index(&heap, v, k);
    printf("size of heap is %d (heap is %sempty)\n",
           rheap_size(&heap),
           rheap_empty(&heap) ? "" : "not ");
  }

  printf("heap layout:\n");
  rheap_print(&heap);
  printf("\n");

  printf("modify 0 3->1\n");
  rheap_modify(&heap, 0, 1);
  printf("heap layout:\n");
  rheap_print(&heap);
  printf("\n");

  printf("modify 0 1->14\n");
  rheap_modify(&heap, 0, 14);
  printf("heap layout:\n");
  rheap_print(&heap);
  printf("\n");

  printf("modify 0 14->3\n");
  rheap_modify(&heap, 0, 3);
  printf("heap layout:\n");
  rheap_print(&heap);
  printf("\n");

  while(!rheap_empty(&heap)) {
    igraph_integer_t key = rheap_min_index(&heap);
    igraph_integer_t value = rheap_delete_min(&heap);
    printf("pop from heap: key=%d, value=%d\n", key, value);
    printf("size of heap is %d (heap is %sempty)\n",
           rheap_size(&heap),
           rheap_empty(&heap) ? "" : "not ");
    printf("heap layout:\n");
    rheap_print(&heap);
    printf("\n");
  }

  rheap_destroy(&heap);

  return 0;
}
