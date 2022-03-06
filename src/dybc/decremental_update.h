
#ifndef _DYNAMIC_DECREMENTAL_UPDATE_
#define _DYNAMIC_DECREMENTAL_UPDATE_

#include <igraph/igraph.h>
#include "dybc/dynamic_betweenness_util.h"

void decremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        dybc_update_stats_t* upd_stats);

#endif // _DYNAMIC_DECREMENTAL_UPDATE_
