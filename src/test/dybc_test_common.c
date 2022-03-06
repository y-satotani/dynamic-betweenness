
#include "dybc_test_common.h"

#include <stdio.h>
#include <igraph/igraph.h>
#include "core/math.h"

#include "dybc/static_betweenness.h"
#include "dybc/incremental_update.h"
#include "dybc/decremental_update.h"

int check_quantities(const char* test_name,
                     igraph_t* G,
                     igraph_matrix_t* D,
                     igraph_matrix_int_t* S,
                     igraph_vector_t* B,
                     igraph_vector_t* weights) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define B(a) (VECTOR(*B)[(a)])
#define dt(a, b) (MATRIX(D_true, (a), (b)))
#define st(a, b) (MATRIX(S_true, (a), (b)))
#define Bt(a) (VECTOR(B_true)[(a)])
#define Bg(a) (VECTOR(B_grand)[(a)])

  igraph_matrix_t D_true;
  igraph_matrix_int_t S_true;
  igraph_vector_t B_true;
  igraph_vector_t B_grand;

  igraph_matrix_init(&D_true, igraph_vcount(G), igraph_vcount(G));
  igraph_matrix_int_init(&S_true, igraph_vcount(G), igraph_vcount(G));
  igraph_vector_init(&B_true, igraph_vcount(G));
  igraph_vector_init(&B_grand, igraph_vcount(G));
  betweenness_with_redundant_information(G, &D_true, &S_true, &B_true, weights);
  igraph_betweenness
    (G, &B_grand, igraph_vss_all(), igraph_is_directed(G), weights);
  if(!igraph_is_directed(G))
    igraph_vector_scale(&B_grand, 2);

  int dist_err = 0, sigma_err = 0, bet_err = 0;
  // check distance
  for(long int s = 0; s < igraph_vcount(G); s++) {
    for(long int t = 0; t < igraph_vcount(G); t++) {
      if(cmp(d(s, t), dt(s, t))) {
        if(!dist_err) {
          printf("distance error on %s:\n", test_name);
          dist_err = 1;
        }
        printf("%ld %ld (res:%f true:%f)\n", s, t, d(s, t), dt(s, t));
      }
    }
  }
  // check geodesics
  for(long int s = 0; s < igraph_vcount(G); s++) {
    for(long int t = 0; t < igraph_vcount(G); t++) {
      if(s(s, t) != st(s, t)) {
        if(!sigma_err) {
          printf("geodesics error on %s:\n", test_name);
          sigma_err = 1;
        }
        printf("%ld %ld (res:%d true:%d)\n", s, t, s(s, t), st(s, t));
      }
    }
  }
  // check betweenness
  for(long int x = 0; x < igraph_vcount(G); x++) {
    igraph_real_t diff1 = fabs(B(x) - Bt(x));
    igraph_real_t diff2 = fabs(B(x) - Bg(x));
    if(cmp(diff1, EPS) > 0 || cmp(diff2, EPS) > 0) {
      if(!bet_err) {
        printf("betweenness error on %s:\n", test_name);
        bet_err = 1;
      }
      printf("%ld (res:%g true:%g grand:%g)\n", x, B(x), Bt(x), Bg(x));
    }
  }

  igraph_matrix_destroy(&D_true);
  igraph_matrix_int_destroy(&S_true);
  igraph_vector_destroy(&B_true);
  igraph_vector_destroy(&B_grand);
#undef EPS
#undef cmp
#undef d
#undef dt
#undef s
#undef st
#undef B
#undef Bt
#undef Bg
  return !(dist_err | sigma_err | bet_err);
}

void increment(igraph_t* G,
               igraph_matrix_t* D,
               igraph_matrix_int_t *S,
               igraph_vector_t* B,
               igraph_integer_t u,
               igraph_integer_t v,
               igraph_vector_t* weights,
               igraph_real_t weight) {
  incremental_update(G, D, S, B, u, v, weights, weight, NULL);
  // add edge
  igraph_add_edge(G, u, v);
  if(weights)
    igraph_vector_push_back(weights, weight);
}

void decrement(igraph_t* G,
               igraph_matrix_t* D,
               igraph_matrix_int_t *S,
               igraph_vector_t* B,
               igraph_integer_t u,
               igraph_integer_t v,
               igraph_vector_t* weights,
               igraph_real_t weight) {
  decremental_update(G, D, S, B, u, v, weights, weight, NULL);
  // delete an edge
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, igraph_is_directed(G), 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  if(weights)
    igraph_vector_remove(weights, eid);
}


