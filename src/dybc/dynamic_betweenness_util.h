#ifndef _DYNAMIC_BETWEENNESS_UTIL_H_
#define _DYNAMIC_BETWEENNESS_UTIL_H_

#include <igraph/igraph.h>

typedef enum dybc_update_query_t {
  QUERY_NONE,
  QUERY_INSERT,
  QUERY_DELETE
} dybc_update_query_t;

typedef struct dybc_update_stats_t {
  double upd_betw;
  double upd_path;
  unsigned long n_tau_hat;
  unsigned long n_aff_src;
  unsigned long n_aff_tgt;
  double time_betw;
  double time_path;
  double time_full;
} dybc_update_stats_t;


void count_affected_vertices_betw(igraph_t* G,
                                  igraph_vector_int_t* aff_deps_before,
                                  igraph_vector_int_t* aff_deps_after,
                                  igraph_vector_int_t* sources,
                                  dybc_update_stats_t* upd_stats);

#endif // _DYNAMIC_BETWEENNESS_UTIL_H_
