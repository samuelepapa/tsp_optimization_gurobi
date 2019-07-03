#ifndef TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H

#include "common.h"
#include "input_output.h"
#include "utils.h"
#include "limits.h"

int two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs);

int two_opt_f(Tsp_prob *instance, int *node_sequence, int *costs);

int two_opt_dlb(Tsp_prob *instance, double *solution, int *node_sequence, int *costs);

void double_bridge_kick(Tsp_prob *instance, int *output_sequence, int n_node, int *input_sequence);

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance);

int compute_total_distance(Tsp_prob *instance, int *node_sequence);

void kick(Tsp_prob *instance, int *node_sequence, int n_node, int *incumbent_node_sequence);

void new_solution(Tsp_prob *instance, int *input_sequence, double *output_solution);

void copy_solution(Tsp_prob *instance, double *input_solution, double *output_solution);

void copy_node_sequence(int *to_node_sequence, int *from_node_sequence, int n_node);

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence);

#endif //TSP_OPTIMIZATION_GUROBI_META_HEURISTIC_UTILS_H
