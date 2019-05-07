#ifndef TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H
#define TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H

#include "common.h"
#include "utils.h"
#include "tsp_lazycall.h"
#include "tsp_loop.h"
#include "tsp_usercall.h"


void tsp_hardfixing_model_create(Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H
