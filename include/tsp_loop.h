//
// Created by samuele on 26/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_TSP_LOOP_H
#define TSP_OPTIMIZATION_GUROBI_TSP_LOOP_H

#include "common.h"
#include "utils.h"
#include "input_output.h"
#include "union_find.h"

void tsp_loop_model_create(Tsp_prob *instance);

void tsp_loop_model_generate(Tsp_prob *instance);

void tsp_loop_model_run(Tsp_prob *instance);

/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos_loop(int i, int j, Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_LOOP_H
