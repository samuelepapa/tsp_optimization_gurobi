#ifndef TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H

#include "common.h"
#include "input_output.h"
#include "utils.h"
#include "limits.h"

int two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs);

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance);

int compute_total_distance(Tsp_prob *instance, int *node_sequence);

void kick(Tsp_prob *instance, double *solution, int n_node, double *input_solution);

void new_solution(Tsp_prob *instance, int *input_sequence, double *output_solution);

int random_two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs);

#endif //TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
