//
// Created by samuele on 07/05/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H

#include "common.h"
#include "utils.h"

/**
 * Inverse of map_warm_start_type, target_string needs to have been previously initialized
 * @param model_type the index of the initial heuristic solution method
 * @param target_string the output buffer for the mnemonic name, needs to have been allocated before passing it
 */
void inverse_map_warm_start_type(int model_type, char *target_string);

/**
 * Used to convert mnemonic name of the initial heuristic solution method with the index
 * @param optarg the name of the initial heuristic solution method
 * @return the index of the initial heuristic solution method
 */
int map_warm_start_type(char *optarg);

/**
 * Computes the initial heuristic solution according to the warm_start paramenter in instance, returns it in the
 * pointer solution
 * @param instance the tsp_prob instance where the type of initial solution computation is defined
 * @param solution the solution found using warm_start method
 * @param var_pos the function to compute the index of the variable (used to populate solution)
 */
void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));

/**
 * Set the warm start in the current model, uses get_initial_heuristic_sol to find the first solution to feed the
 * model, feeds it to it and sets the solution limit to 2. Remeber to set it back to infinity after having performed
 * the first cycle (if the first cycle was necessary)
 * @param instance the tsp_prob instance
 * @param var_pos function to find the index of the variable
 */
void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

#endif //TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H
