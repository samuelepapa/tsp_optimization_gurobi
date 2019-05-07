//
// Created by samuele on 07/05/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H

#include "common.h"
#include "utils.h"

void inverse_map_warm_start_type(int model_type, char *target_string);

int map_warm_start_type(char *optarg);

void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));

void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

#endif //TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H
