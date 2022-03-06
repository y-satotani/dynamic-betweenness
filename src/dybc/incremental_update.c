
#include "incremental_update.h"

#include <time.h>
#include <igraph/igraph.h>
#include "core/math.h"

#include "dybc/dynamic_betweenness_util.h"
#include "dybc/incremental_shortest_path.h"
#include "dybc/incremental_betweenness.h"

void incremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        dybc_update_stats_t* upd_stats) {
  // initialize the edgelists of the graph
  clock_t start, end;
  igraph_inclist_t succs, preds;
  start = clock();
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT, IGRAPH_NO_LOOPS);
    igraph_inclist_init(G, &preds, IGRAPH_IN, IGRAPH_NO_LOOPS);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL, IGRAPH_NO_LOOPS);
    preds = succs;
  }
  end = clock();
  if(upd_stats) {
    upd_stats->time_full = (double)(end - start) / CLOCKS_PER_SEC;
    upd_stats->time_path = 0.0;
    upd_stats->time_betw = 0.0;
  }

  count_affected_vertices_path_inc
    (G, &preds, &succs, D, S, B, u, v, weights, weight, upd_stats, 0);

  // initialize vectors store affected vertices
  igraph_vector_int_t sources, targets;
  igraph_vector_int_t aff_deps_before, aff_deps_after;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&aff_deps_before, 0);
  igraph_vector_int_init(&aff_deps_after, 0);

  // find affected sources/targets
  start = clock();
  affected_sources_inc
    (G, &preds, &sources, D, u, v, v, weights, weight, 0);
  end = clock();
  if(upd_stats)
    upd_stats->time_full += (double)(end - start) / CLOCKS_PER_SEC;

  // initialize cache
  igraph_vector_t d_old;
  igraph_vector_int_t s_old;
  igraph_vector_bool_t is_affected;
  igraph_vector_init(&d_old, igraph_vcount(G));
  igraph_vector_int_init(&s_old, igraph_vcount(G));
  igraph_vector_bool_init(&is_affected, igraph_vcount(G));
  void* queue_sssp;
  void* queue_betw;
  igraph_vector_t Delta;
  igraph_vector_bool_init(&is_affected, igraph_vcount(G));
  if(weights) {
    queue_sssp = malloc(sizeof(igraph_2wheap_t));
    queue_betw = malloc(sizeof(igraph_2wheap_t));
    igraph_2wheap_init((igraph_2wheap_t*)queue_sssp, igraph_vcount(G));
    igraph_2wheap_init((igraph_2wheap_t*)queue_betw, igraph_vcount(G));
  } else {
    queue_sssp = malloc(sizeof(igraph_dqueue_t));
    queue_betw = malloc(sizeof(rheap_t));
    igraph_dqueue_init((igraph_dqueue_t*)queue_sssp, igraph_vcount(G));
    rheap_init((rheap_t*)queue_betw, igraph_vcount(G), 0);
  }
  igraph_vector_init(&Delta, igraph_vcount(G));

  // for each vertex in affected sources
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    start = clock();
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc
      (G, &succs, &targets, D, u, v, s, weights, weight, 0);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_betw_inc_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, -factor, 0,
         &is_affected, (igraph_2wheap_t*)queue_betw, &Delta, &aff_deps_before);
    else
      update_betw_inc_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, -factor, 0,
         &is_affected, (rheap_t*)queue_betw, &Delta, &aff_deps_before);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // update sssp
    start = clock();
    if(weights)
      update_sssp_inc_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight, 0,
         &d_old, &s_old, (igraph_2wheap_t*)queue_sssp);
    else
      update_sssp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, s, 0,
         &d_old, &s_old, (igraph_dqueue_t*)queue_sssp, &is_affected);
    end = clock();
    if(upd_stats) {
      double time_path = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_path;
      upd_stats->time_path += time_path;
    }

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    start = clock();
    if(weights)
      update_betw_inc_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, factor, 1,
         &is_affected, (igraph_2wheap_t*)queue_betw, &Delta, &aff_deps_after);
    else
      update_betw_inc_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, factor, 1,
         &is_affected, (rheap_t*)queue_betw, &Delta, &aff_deps_after);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

  } // for s in sources

  count_affected_vertices_betw
    (G, &aff_deps_before, &aff_deps_after, &sources, upd_stats);

  // cleanup
  igraph_vector_destroy(&d_old);
  igraph_vector_int_destroy(&s_old);
  igraph_vector_destroy(&Delta);
  igraph_vector_bool_destroy(&is_affected);
  if(weights) {
    igraph_2wheap_destroy((igraph_2wheap_t*)queue_sssp);
    igraph_2wheap_destroy((igraph_2wheap_t*)queue_betw);
  } else {
    igraph_dqueue_destroy((igraph_dqueue_t*)queue_sssp);
    rheap_destroy((rheap_t*)queue_betw);
  }
  free(queue_sssp);
  free(queue_betw);

  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&aff_deps_before);
  igraph_vector_int_destroy(&aff_deps_after);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
