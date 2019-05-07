#ifndef TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H
#define TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H

#include "common.h"
#include "utils.h"
#include "tsp_lazycall.h"
#include "tsp_loop.h"
#include "tsp_usercall.h"


void tsp_matheuristic_model_create(Tsp_prob *instance);

int change_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *), double cur_sol, double *solution, double node_cnt, int num_conn_comp, Connected_component *conn_comps);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_MATHEURISTIC_H
