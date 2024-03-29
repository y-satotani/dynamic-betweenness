#include "decremental_betweenness.h"

#include <igraph/igraph.h>
#include <limits.h>
#include "core/math.h"
#include "core/buckets.h"
#include "core/indheap.h"
#include "rheap.h"

void update_betw_dec_weighted(igraph_t* G,
                              igraph_inclist_t* preds,
                              igraph_matrix_t* D,
                              igraph_matrix_int_t* S,
                              igraph_vector_t* B,
                              igraph_integer_t u,
                              igraph_integer_t v,
                              igraph_integer_t source,
                              igraph_vector_int_t* targets,
                              igraph_vector_t* weights,
                              igraph_real_t weight,
                              igraph_real_t factor,
                              igraph_bool_t is_post_update,
                              igraph_vector_bool_t* is_target,
                              igraph_2wheap_t* queue,
                              igraph_vector_t* Delta) {
  update_betw_dec_weighted_statistics
    (G, preds, D, S, B, u, v, source, targets, weights, weight, factor, is_post_update, is_target, queue, Delta, 0);
}

void update_betw_dec_unweighted(igraph_t* G,
                                igraph_inclist_t* preds,
                                igraph_matrix_t* D,
                                igraph_matrix_int_t* S,
                                igraph_vector_t* B,
                                igraph_integer_t u,
                                igraph_integer_t v,
                                igraph_integer_t source,
                                igraph_vector_int_t* targets,
                                igraph_real_t factor,
                                igraph_bool_t is_post_update,
                                igraph_vector_bool_t* is_target,
                                rheap_t* queue,
                                igraph_vector_t* Delta) {
  update_betw_dec_unweighted_statistics
    (G, preds, D, S, B, u, v, source, targets, factor, is_post_update, is_target, queue, Delta, 0);
}

void update_betw_dec_weighted_statistics(igraph_t* G,
                                         igraph_inclist_t* preds,
                                         igraph_matrix_t* D,
                                         igraph_matrix_int_t* S,
                                         igraph_vector_t* B,
                                         igraph_integer_t u,
                                         igraph_integer_t v,
                                         igraph_integer_t source,
                                         igraph_vector_int_t* targets,
                                         igraph_vector_t* weights,
                                         igraph_real_t weight,
                                         igraph_real_t factor,
                                         igraph_bool_t is_post_update,
                                         igraph_vector_bool_t* is_target,
                                         igraph_2wheap_t* queue,
                                         igraph_vector_t* Delta,
                                         igraph_vector_int_t* traversed_vertices) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)
#define B(a) (VECTOR(*B)[(a)])
#define Delta(a) (VECTOR(*Delta)[(a)])

  igraph_2wheap_clear(queue);
  igraph_vector_bool_fill(is_target, 0);
  igraph_vector_fill(Delta, 0);
  for(long int ti = 0; ti < igraph_vector_int_size(targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(targets, ti);
    if(igraph_is_inf(d(source, t))) continue;
    igraph_2wheap_push_with_index(queue, t, d(source, t));
    igraph_vector_bool_set(is_target, t, 1);
  }

  while(!igraph_2wheap_empty(queue)) {
    igraph_integer_t x = igraph_2wheap_max_index(queue);
    igraph_2wheap_delete_max(queue);
    B(x) += factor * Delta(x);
    if(traversed_vertices)
      igraph_vector_int_push_back(traversed_vertices, x);
    if(igraph_is_inf(d(source, x))) continue;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(is_post_update && x == v && y == u) continue;

      if(y == source || cmp(d(source, x), d(source, y) + l(eid)) < 0)
        continue;
      if(VECTOR(*is_target)[x])
        Delta(y) += (1. + Delta(x)) * s(source, y) / s(source, x);
      else
        Delta(y) += Delta(x) * s(source, y) / s(source, x);
      if(!igraph_2wheap_has_elem(queue, y))
        igraph_2wheap_push_with_index(queue, y, d(source, y));
    }

  }

#undef EPS
#undef cmp
#undef d
#undef s
#undef l
#undef B
#undef Delta
}

void update_betw_dec_unweighted_statistics(igraph_t* G,
                                           igraph_inclist_t* preds,
                                           igraph_matrix_t* D,
                                           igraph_matrix_int_t* S,
                                           igraph_vector_t* B,
                                           igraph_integer_t u,
                                           igraph_integer_t v,
                                           igraph_integer_t source,
                                           igraph_vector_int_t* targets,
                                           igraph_real_t factor,
                                           igraph_bool_t is_post_update,
                                           igraph_vector_bool_t* is_target,
                                           rheap_t* queue,
                                           igraph_vector_t* Delta,
                                           igraph_vector_int_t* traversed_vertices) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define B(a) (VECTOR(*B)[(a)])
#define Delta(a) (VECTOR(*Delta)[(a)])

  // assumes that `target` is sorted in accending order of distance
  igraph_vector_bool_fill(is_target, 0);
  rheap_clear(queue, igraph_vcount(G));
  igraph_vector_fill(Delta, 0);
  for(long int ti = 0; ti < igraph_vector_int_size(targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(targets, ti);
    if(igraph_is_inf(d(source, t))) continue;
    rheap_push_with_index(queue, t, -(int)d(source, t));
    igraph_vector_bool_set(is_target, t, 1);
  }

  while(!rheap_empty(queue)) {
    igraph_integer_t x = rheap_delete_min(queue);
    B(x) += factor * Delta(x);
    if(traversed_vertices)
      igraph_vector_int_push_back(traversed_vertices, x);
    if(igraph_is_inf(d(source, x))) continue;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(is_post_update && x == v && y == u) continue;

      if(y == source || cmp(d(source, x), d(source, y) + 1.0) < 0)
        continue;
      if(VECTOR(*is_target)[x])
        Delta(y) += (1. + Delta(x)) * s(source, y) / s(source, x);
      else
        Delta(y) += Delta(x) * s(source, y) / s(source, x);
      if(!rheap_has_elem(queue, y)) {
        rheap_push_with_index(queue, y, -(int)d(source, y));
      }
    }
  }

#undef EPS
#undef cmp
#undef d
#undef s
#undef B
#undef Delta
}
