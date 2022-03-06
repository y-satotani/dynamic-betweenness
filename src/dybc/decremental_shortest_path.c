#include "decremental_shortest_path.h"

#include <assert.h>
#include <igraph/igraph.h>
#include "core/math.h"
#include "core/indheap.h"
#include "rheap.h"

void update_sssp_dec_weighted(igraph_t* G,
                              igraph_inclist_t* preds,
                              igraph_inclist_t* succs,
                              igraph_matrix_t* D,
                              igraph_matrix_int_t* S,
                              igraph_integer_t u,
                              igraph_integer_t v,
                              igraph_integer_t source,
                              igraph_vector_int_t* targets,
                              igraph_vector_t* weights,
                              igraph_real_t weight,
                              igraph_bool_t is_post_mod,
                              igraph_vector_bool_t* is_affected,
                              igraph_2wheap_t* queue) {

  // edge getting modification
  igraph_integer_t eid_;
  igraph_get_eid(G, &eid_, u, v, 1, 0);
  assert(eid_ >= 0 && "edge does not exist");

#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)
#define lp(eid) (eid == eid_ ? weight : (weights ? VECTOR(*weights)[eid] : 1))

  // early out: no effect if d_sv < d_su + l_uv
  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + l(eid_)) < 0) {
    return;
  }

  igraph_vector_bool_fill(is_affected, 0);
  for(long int ti = 0; ti < igraph_vector_int_size(targets); ti++)
    VECTOR(*is_affected)[igraph_vector_int_e(targets, ti)] = 1;

  igraph_2wheap_clear(queue);
  for(long int xi = 0; xi < igraph_vector_int_size(targets); xi++) {
    igraph_integer_t x = igraph_vector_int_e(targets, xi);
    igraph_vector_int_t* ys = igraph_inclist_get(preds, x);
    igraph_real_t d_hat = d(source, x) + weight - l(eid_);
    for(long int yi = 0; yi < igraph_vector_int_size(ys); yi++) {
      igraph_integer_t eid = igraph_vector_int_e(ys, yi);
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(!VECTOR(*is_affected)[y] && cmp(d(source, y) + lp(eid), d_hat) < 0)
        d_hat = d(source, y) + lp(eid);
    }
    if(igraph_is_inf(d_hat)) {
      d(source, x) = IGRAPH_INFINITY;
      s(source, x) = 0;
      if(!igraph_is_directed(G)) {
        d(x, source) = IGRAPH_INFINITY;
        s(x, source) = 0;
      }
    } else {
      igraph_2wheap_push_with_index(queue, x, -d_hat);
    }
  }

  while(!igraph_2wheap_empty(queue)) {
    long int x = igraph_2wheap_max_index(queue);
    d(source, x) = -igraph_2wheap_delete_max(queue);
    s(source, x) = 0;
    if(!igraph_is_directed(G)) {
      d(x, source) = d(source, x);
      s(x, source) = 0;
    }
    VECTOR(*is_affected)[x] = 0;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t p = IGRAPH_OTHER(G, eid, x);
      if(!VECTOR(*is_affected)[p]
         && cmp(d(source, x), d(source, p) + lp(eid)) == 0) {
        s(source, x) += s(source, p);
        if(!igraph_is_directed(G))
          s(x, source) += s(p, source);
      }
    } /* for preds */

    igraph_vector_int_t* ss = igraph_inclist_get(succs, x);
    for(long int si = 0; si < igraph_vector_int_size(ss); si++) {
      igraph_integer_t eid = VECTOR(*ss)[si];
      igraph_integer_t s = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_s = d(source, x) + lp(eid);
      if(VECTOR(*is_affected)[s]) {
        if(!igraph_2wheap_has_elem(queue, s))
          igraph_2wheap_push_with_index(queue, s, -d_s);
        else if(cmp(d_s, -igraph_2wheap_get(queue, s)) < 0)
          igraph_2wheap_modify(queue, s, -d_s);
      }
    } /* for succs */
  } /* !igraph_2wheap_empty(&Q) */

#undef EPS
#undef cmp
#undef d
#undef s
#undef l
#undef lp
}

void update_sssp_dec_unweighted(igraph_t* G,
                                igraph_inclist_t* preds,
                                igraph_inclist_t* succs,
                                igraph_matrix_t* D,
                                igraph_matrix_int_t* S,
                                igraph_integer_t u,
                                igraph_integer_t v,
                                igraph_integer_t source,
                                igraph_vector_int_t* targets,
                                igraph_bool_t is_post_mod,
                                igraph_vector_bool_t* is_affected,
                                rheap_t* queue) {

  // edge getting modification
  igraph_integer_t eid_;
  igraph_get_eid(G, &eid_, u, v, 1, 0);
  assert(eid_ >= 0 && "edge does not exist");

#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) 1.0
#define lp(eid) (eid == eid_ ? IGRAPH_INFINITY : 1.0)

  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + l(eid_)) < 0) {
    return;
  }

  igraph_vector_bool_fill(is_affected, 0);
  for(long int xi = 0; xi < igraph_vector_int_size(targets); xi++)
    VECTOR(*is_affected)[VECTOR(*targets)[xi]] = 1;

  rheap_clear(queue, 0);
  for(long int xi = 0; xi < igraph_vector_int_size(targets); xi++) {
    igraph_integer_t x = VECTOR(*targets)[xi];
    igraph_vector_int_t* ys = igraph_inclist_get(preds, x);
    igraph_real_t d_hat = d(source, x) + lp(eid_) - l(eid_);
    igraph_integer_t s_hat = 0;
    for(long int yi = 0; yi < igraph_vector_int_size(ys); yi++) {
      igraph_integer_t eid = igraph_vector_int_e(ys, yi);
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(VECTOR(*is_affected)[y]) continue;
      if(cmp(d(source, y) + lp(eid), d_hat) < 0) {
        d_hat = d(source, y) + lp(eid);
        s_hat = 0;
      }
      if(cmp(d(source, y) + lp(eid), d_hat) == 0)
        s_hat += s(source, y);
    } // end for each y in neighbor of x
    if(!igraph_is_inf(d_hat)) {
      rheap_push_with_index(queue, x, (int)d_hat);
    }
    else
      s_hat = 0;
    d(source, x) = d_hat;
    s(source, x) = s_hat;
    if(!igraph_is_directed(G)) {
      d(x, source) = d_hat;
      s(x, source) = s_hat;
    }
  } // end for each affected target x
  for(long int xi = 0; xi < igraph_vector_int_size(targets); xi++)
    if(!igraph_is_inf(d(source, VECTOR(*targets)[xi])))
      VECTOR(*is_affected)[VECTOR(*targets)[xi]] = 0;

  while(!rheap_empty(queue)) {
    long int x = rheap_delete_min(queue);
    VECTOR(*is_affected)[x] = 0;

    igraph_vector_int_t* neis = igraph_inclist_get(succs, x);
    long int nneis = igraph_vector_int_size(neis);
    for(long int ni = 0; ni < nneis; ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);

      if(cmp(d(source, x) + lp(eid), d(source, y)) < 0) {
        d(source, y) = d(source, x) + lp(eid);
        s(source, y) = s(source, x);
        if(!igraph_is_directed(G)) {
          d(y, source) = d(x, source) + lp(eid);
          s(y, source) = s(x, source);
        }
        if(!rheap_has_elem(queue, y)) {
          rheap_push_with_index(queue, y, (int)d(source, y));
        } else if(cmp((int)d(source, y), rheap_get(queue, y)) < 0) {
          rheap_modify(queue, y, (int)d(source, y));
        }
      } else if(cmp(d(source, x) + lp(eid), d(source, y)) == 0) {
        s(source, y) += s(source, x);
        if(!igraph_is_directed(G)) {
          s(y, source) += s(x, source);
        }
      } // end if(d_sx+l_xy == d_sy)
    } // end for all y in neighbors
  } /* !rheap_empty(&Q) */

#undef EPS
#undef cmp
#undef d
#undef s
#undef l
#undef lp
}

void affected_targets_dec(igraph_t* G,
                          igraph_inclist_t* inclist,
                          igraph_vector_int_t* out,
                          igraph_matrix_t* D,
                          igraph_integer_t u,
                          igraph_integer_t v,
                          igraph_integer_t source,
                          igraph_vector_t* weights,
                          igraph_real_t weight,
                          igraph_bool_t is_post_mod) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)

  // weight setting
  if(!weights) weight = IGRAPH_INFINITY;
  assert(weight > 0);

  // clear output
  assert(out);
  igraph_vector_int_clear(out);

  // early out: no effect if d_sv < d_su + l_uv
  igraph_integer_t eid_;
  igraph_get_eid(G, &eid_, u, v, 1, 0);
  assert(eid_ >= 0 && "edge does not exist");
  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + l(eid_)) < 0) {
    return;
  }

  igraph_vector_bool_t visited;
  igraph_dqueue_int_t queue;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_dqueue_int_init(&queue, 0);
  igraph_vector_bool_set(&visited, v, 1);
  igraph_dqueue_int_push(&queue, v);

  while(!igraph_dqueue_int_empty(&queue)) {
    igraph_integer_t x = igraph_dqueue_int_pop(&queue);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    for(igraph_integer_t ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(source, y);
      igraph_real_t d_sy_p = d(source, x) + l(eid);
      if(cmp(d_sy, d_sy_p) == 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_dqueue_int_push(&queue, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_dqueue_int_destroy(&queue);

#undef EPS
#undef cmp
#undef d
#undef l
}

void affected_sources_dec(igraph_t* G,
                          igraph_inclist_t* inclist,
                          igraph_vector_int_t* out,
                          igraph_matrix_t* D,
                          igraph_integer_t u,
                          igraph_integer_t v,
                          igraph_integer_t target,
                          igraph_vector_t* weights,
                          igraph_real_t weight,
                          igraph_bool_t is_post_mod) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)

  // weight setting
  if(!weights) weight = IGRAPH_INFINITY;
  assert(weight > 0);

  // clear output
  assert(out);
  igraph_vector_int_clear(out);

  // early out: no effect if d_ut < l_uv + d_vt
  igraph_integer_t eid_;
  igraph_get_eid(G, &eid_, u, v, 1, 0);
  assert(eid_ >= 0 && "edge does not exist");
  if(igraph_is_inf(d(v, target))
     || cmp(d(u, target), l(eid_) + d(v, target)) < 0) {
    return;
  }

  // initializing
  igraph_vector_bool_t visited;
  igraph_dqueue_int_t queue;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_dqueue_int_init(&queue, 0);
  igraph_vector_bool_set(&visited, u, 1);
  igraph_dqueue_int_push(&queue, u);

  while(!igraph_dqueue_int_empty(&queue)) {
    igraph_integer_t x = igraph_dqueue_int_pop(&queue);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    for(igraph_integer_t ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(y, target);
      igraph_real_t d_sy_p = l(eid) + d(x, target);
      if(cmp(d_sy, d_sy_p) == 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_dqueue_int_push(&queue, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_dqueue_int_destroy(&queue);

#undef EPS
#undef cmp
#undef d
#undef l
}

void count_affected_vertices_path_dec(igraph_t* G,
                                      igraph_inclist_t* preds,
                                      igraph_inclist_t* succs,
                                      igraph_matrix_t* D,
                                      igraph_matrix_int_t* S,
                                      igraph_vector_t* B,
                                      igraph_integer_t u,
                                      igraph_integer_t v,
                                      igraph_vector_t* weights,
                                      igraph_real_t weight,
                                      dybc_update_stats_t* upd_stats,
                                      igraph_bool_t is_post_mod) {
  if(!upd_stats) return;

  long int n_sources, n_targets;
  long int n_affected_targets, n_affected_sources;
  igraph_vector_int_t sources, targets;
  n_affected_sources = n_affected_targets = 0;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);

  // count targets
  affected_sources_dec
    (G, preds, &sources, D, u, v, v, weights, weight, is_post_mod);
  n_sources = igraph_vector_int_size(&sources);
  for(long int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = VECTOR(sources)[si];
    if(!igraph_is_directed(G))
      affected_sources_dec
        (G, preds, &targets, D, v, u, s, weights, weight, is_post_mod);
    else
      affected_targets_dec
        (G, succs, &targets, D, u, v, s, weights, weight, is_post_mod);
    n_affected_targets += igraph_vector_int_size(&targets);
  }

  // count sources
  if(!igraph_is_directed(G))
    affected_sources_dec
      (G, preds, &targets, D, v, u, u, weights, weight, is_post_mod);
  else
    affected_targets_dec
      (G, succs, &targets, D, u, v, u, weights, weight, is_post_mod);
  n_targets = igraph_vector_int_size(&targets);
  for(long int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = VECTOR(targets)[ti];
    affected_sources_dec
      (G, preds, &sources, D, u, v, t, weights, weight, is_post_mod);
    n_affected_sources += igraph_vector_int_size(&sources);
  }

  // calculate the mean of the number of affected vertices
  if(n_sources + n_targets > 0)
    upd_stats->upd_path
      = (double)(n_affected_sources + n_affected_targets)
      / (n_targets + n_sources);
  else
    upd_stats->upd_path = 0;
  upd_stats->n_aff_src = n_sources;
  upd_stats->n_aff_tgt = n_targets;

  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
}
