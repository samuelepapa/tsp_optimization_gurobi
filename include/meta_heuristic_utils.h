#ifndef TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H

#include "common.h"
#include "utils.h"

void two_opt(Tsp_prob *instance, double *solution, int *node_sequence);

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance);

int compute_total_distance(Tsp_prob *instance, int *node_sequence);

void kick(Tsp_prob *instance, double *solution, int n_node);

void new_solution(Tsp_prob *instance, int *best_solution, double *solution);

int x_pos(int i, int j, Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
