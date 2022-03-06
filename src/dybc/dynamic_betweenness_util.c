#include "dynamic_betweenness_util.h"

#include <igraph/igraph.h>

void count_affected_vertices_betw(igraph_t* G,
                                  igraph_vector_int_t* aff_deps_before,
                                  igraph_vector_int_t* aff_deps_after,
                                  igraph_vector_int_t* sources,
                                  dybc_update_stats_t* upd_stats) {
  if(!upd_stats) return;

  igraph_integer_t n_sources = igraph_vector_int_size(sources);
  if(n_sources > 0)
    upd_stats->upd_betw
      = (double)(igraph_vector_int_size(aff_deps_before)
                 + igraph_vector_int_size(aff_deps_after))
      / (2 * n_sources);
  else
    upd_stats->upd_betw = 0;

  upd_stats->n_tau_hat = 0;
  igraph_vector_bool_t is_affected;
  igraph_vector_bool_init(&is_affected, igraph_vcount(G));
  for(long int i = 0; i < igraph_vector_int_size(aff_deps_before); i++)
    VECTOR(is_affected)[VECTOR(*aff_deps_before)[i]] = 1;
  for(long int i = 0; i < igraph_vector_int_size(aff_deps_after); i++)
    VECTOR(is_affected)[VECTOR(*aff_deps_after)[i]] = 1;
  for(long int i = 0; i < igraph_vector_bool_size(&is_affected); i++)
    upd_stats->n_tau_hat += VECTOR(is_affected)[i];
  igraph_vector_bool_destroy(&is_affected);
}
