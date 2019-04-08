//
// Created by samuele on 07/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_UTILS_H


/**
 * Compute the distance between two points in two dimensions with the method described in the weight_type value of instance
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The distance value from i to j
 */
int distance(int i, int j, Tsp_prob *instance);

/**
 * Free memory to avoid leaks, assumes instance is initialized as variable, not dinamically allocated
 * @param instance The pointer to the problem instance
 */
void close_instance(Tsp_prob *instance);

/**
 * Print the error message associated by error integer value and free the gurobi model and the gurobi environment
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param error Integer error value returned by the gurobi methods
 */
void quit_on_GRB_error(GRBenv *env, GRBmodel *model, int error);

/**
 * Free the gurobi model and the gurobi environment
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 */
void free_gurobi(GRBenv *env, GRBmodel *model);

void inverse_map_model_type (int model_type, char *target_string);
/**
 * Map model type string value in integer value
 * @param optarg Pointer to the value associated to -m value in input
 * @return Integer value to the string
 *
 * The integer values are:
 * 0 for the standard TSP problem with subtour elimination constraints
 * 1 for the TSP problem with Miller, Tucker and Zemlin (MTZ) method
 * 2 for the TSP problem with Matteo Fischetti lecture method
 */
int map_model_type (char *optarg);

void close_trial(Trial *trial_inst);

/**
 * Find connected components returned by the MIP solver
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param instance The pointer to the problem instance
 * @param comp The pointer to the connected component structure
 */
void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp,
                          int (*var_pos)(int, int, Tsp_prob *));

/**
 * Free memory allocated to connected component elements
 * @param comp The pointer to the connected component structure
 */
void free_comp_struc(Connected_comp * comp);

/**
 * Free memory allocated to the graph
 * @param graph The pointer to the graph structure
 */
void free_graph(Graph *graph);


#endif //TSP_OPTIMIZATION_GUROBI_UTILS_H
