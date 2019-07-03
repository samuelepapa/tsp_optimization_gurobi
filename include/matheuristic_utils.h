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
void set_warm_start_heu(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

void set_warm_start(Tsp_prob *instance);

/**
 * Simple initial heuristic solution which just links all the nodes in order
 * @param instance the tsp_prob instance
 * @param solution the solution where to write the output
 * @param var_pos function used to map edge notation (i,j) to variable index notation according to the notation used
 */
void simple_initial_heuristic_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));

void naive_initial_heuristic_solution(int start_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *));

void grasp_initial_heuristic_solution(int start_node, double p_greedy, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *));

/**
 *
 * @param first_node
 * @param second_node
 * @param p_grasp
 * @param instance
 * @param solution
 * @param var_pos
 */
void extra_mileage_initial_heuristic_solution(int first_node, int second_node, double p_grasp, Tsp_prob *instance,
                                              double *solution, int (*var_pos)(int, int, Tsp_prob *));

int x_pos_metaheuristic(int i, int j, Tsp_prob *instance);


#endif //TSP_OPTIMIZATION_GUROBI_MATHEURISTIC_UTILS_H
