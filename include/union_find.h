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
    Edge *edge;
} Graph;

/**
 * Structure to represent the connected component in the graph
 */
/*typedef struct {
    //the root of the element in the subset tree
    int parent;
    //rank of element in the subset
    int rank;
    //size of the connected component
    int size;
} Connected_component;*/

typedef struct {
    //the root of the element in the subset tree
    int *parent;
    //rank of element in the subset
    int *rank;
    //size of the connected component
    int *size;
} Connected_component;

/**
 * Find connected components in the graph using union-find algorithm with path halving technique
 * @param graph The pointer to the graph representation structure
 * @param solution The pointer to the solver solution
 * @param var_pos The pointer to the mapping method between user variables and Gurobi variables
 * @param instance The pointer to the problem instance
 * @param conn_comps The pointer to the connected components structure
 * @return The number of connected components
 */

int union_find(Graph *graph, double *solution, int (*var_pos)(int, int, Tsp_prob *), Tsp_prob *instance, Connected_component *conn_comps);

/**
 * Initialize the graph for the union find algorithm
 * @param instance The pointer to the problem instance
 */
void create_graph_u_f(Tsp_prob *instance, Graph *graph);

/**
 * Return the connected component roots
 * @param root_cc The pointer to the array of connected component roots
 * @param number_of_comps The number of connected components
 * @param conn_comps The pointer to the connected component structure
 * @param n_node The number of nodes of the problem
 */
void get_root(int *root_cc, int number_of_comps, Connected_component *conn_comps, int n_node);

/**
 * Function to find the set of an element i using path compression technique
 * @param conn_comps The pointer to the connected components structure
 * @param i The node
 * @return The root of the subset tree
 */
int find(Connected_component *conn_comps, int i);

/**
 * Function to find the root of a node in a connected component
 * @param conn_comps The pointer to the connected components structure
 * @param i The node
 * @return The root of the node
 */
int find_root(Connected_component *conn_comps, int i);

void free_comp(Connected_component * conn_comp);

#endif //TSP_OPTIMIZATION_GUROBI_UNION_FIND_H
