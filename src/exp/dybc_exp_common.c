
#include "dybc_exp_common.h"

#include <stdio.h>
#include <time.h>
#include <igraph/igraph.h>
#include "core/math.h"

#include "dybc/dynamic_betweenness_util.h"

int choose_random_edge_for(igraph_t* G,
                           dybc_update_query_t query,
                           igraph_integer_t *u,
                           igraph_integer_t *v,
                           unsigned long int seed) {
  igraph_rng_seed(igraph_rng_default(), seed);
  if(query == QUERY_INSERT) {
    igraph_integer_t eid;
    igraph_t C;
    igraph_complementer(&C, G, 0);
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&C)-1);
    igraph_edge(&C, eid, u, v);
    igraph_destroy(&C);
    return 1;
  } else if(query == QUERY_DELETE) {
    igraph_integer_t eid;
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(G)-1);
    igraph_edge(G, eid, u, v);
    return 1;
  } else {
    *u = *v = -1;
    return 0;
  }
}

