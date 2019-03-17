//
// Created by samuele on 07/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_UTILS_H
#define TSP_OPTIMIZATION_GUROBI_UTILS_H


/**
 * Associative function between points of an edge and memory position
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos(int i, int j, Tsp_prob * instance);

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
 * Adds an edge to the solution array. If the size is negative an error is triggered, and exits with 1.
 * @param instance The Tsp_prob instance where to add the edge.
 * @param edge The edge to be added, allocate memory before passing the pointer.
 */
void add_edge_to_solution(Tsp_prob * instance, int * edge);


void quit_on_GRB_error(GRBenv *env, GRBmodel *model, int error);


#endif //TSP_OPTIMIZATION_GUROBI_UTILS_H
