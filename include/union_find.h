#ifndef TSP_OPTIMIZATION_GUROBI_UNION_FIND_H
#define TSP_OPTIMIZATION_GUROBI_UNION_FIND_H

#include "common.h"

/**
 * Structure to represent an edge in the graph
 */
typedef struct {
    int src;
    int dest;
} Edge;

/**
 * Structure to represent a graph
 */
typedef struct {
    //number of node
    int V;
    //number of edge
    int E;

    //Graph represented as an array of edges
    Edge* edge;
} Graph;

/**
 * Structure to represent the connected component in the graph
 */
typedef struct {
    //the root of the element in the subset tree
    int parent;
    //number of element in the subset
    int rank;
    //row index in the constraints matrix
    int index;
} Connected_component;

/**
 * Find connected components in the graph using union-find algorithm with path compression technique
 * @param graph The pointer to the graph representation structure
 * @param solution The pointer to the solver solution
 * @param var_pos The pointer to the mapping method between user variables and Gurobi variables
 * @param instance The pointer to the problem instance
 * @param subsets The array of connected components
 * @return The number of connected components
 */
int union_find(Graph *graph, double *solution, int (*var_pos)(int, int, Tsp_prob *), Tsp_prob *instance, Connected_component conn_comps[]);

/**
 * Initialize the graph for the union find algorithm
 * @param instance The pointer to the problem instance
 */
void create_graph_u_f(Tsp_prob *instance, Graph *graph);

/**
 * Return the connected component roots
 * @param root_cc The array of connected component roots
 * @param number_of_comps The number of connected components
 * @param conn_comps The array of nodes with connected component data
 * @param n_node The number of nodes of the problem
 */
void get_root(int root_cc[], int number_of_comps, Connected_component conn_comps[], int n_node);

#endif //TSP_OPTIMIZATION_GUROBI_UNION_FIND_H
