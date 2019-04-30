#ifndef TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_H
#define TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_H

#include "common.h"
#include "tsp_lazycall.h"
#include "tsp_loop.h"

void tsp_matheuristic_model_create(Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_H
