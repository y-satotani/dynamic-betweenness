
#ifndef _DYBC_EXP_COMMON_
#define _DYBC_EXP_COMMON_

#include <stdio.h>
#include <igraph/igraph.h>
#include "dybc/dynamic_betweenness_util.h"

int choose_random_edge_for(igraph_t* G,
                           dybc_update_query_t query,
                           igraph_integer_t *u,
                           igraph_integer_t *v,
                           unsigned long int seed);

#endif // _DYBC_EXP_COMMON_
