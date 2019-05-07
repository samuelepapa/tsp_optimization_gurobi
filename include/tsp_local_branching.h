#ifndef TSP_OPTIMIZATION_GUROBI_TSP_LOCAL_BRANCHING_H
#define TSP_OPTIMIZATION_GUROBI_TSP_LOCAL_BRANCHING_H

#include "common.h"
#include "utils.h"
#include "tsp_lazycall.h"
#include "tsp_loop.h"
#include "tsp_usercall.h"
#include "matheuristic_utils.h"

void tsp_local_branching_create(Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_LOCAL_BRANCHING_H
