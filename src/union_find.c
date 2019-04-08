#include <stdio.h>
#include <stdlib.h>
#include "union_find.h"
#include "common.h"
#include "utils.h"

/**
 * Function to find the set of an element i using path compression technique
 * @param conn_comps Array of subsets
 * @param i The element
 * @return The root of the subset tree
 */
int find(Connected_component conn_comps[], int i);

/**
 * Function that does the union of two sets of x and y using union by rank
 * @param conn_comps Array of connected components (subsets)
 * @param x_set First subset
 * @param y_set Second subset
 */
void union_by_rank(Connected_component conn_comps[], int x_set, int y_set);

void create_graph_u_f(Tsp_prob *instance, Graph *graph) {
    int n_node = instance->nnode;
    graph-> V = n_node; //number of nodes
    graph->E = (n_node * (n_node - 1)) / 2; //number of edges
    graph->edge = (Edge*) calloc(graph->E, sizeof(Edge));

    int l = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            graph->edge[l].src = i;
            graph->edge[l].dest = j;
            l++;
        }
    }
}

int union_find(Graph *graph, double *solution, int (*var_pos)(int, int, Tsp_prob *), Tsp_prob *instance, Connected_component conn_comps[]) {

    int n_node = graph->V;
    int n_edge = graph->E;

    int number_of_comps = n_node;

    for (int v = 0; v < n_node; v++) {
        conn_comps[v].parent = v;
        conn_comps[v].rank = 0;
    }

    for (int e = 0; e < n_edge; e++) {

        int src = graph->edge[e].src;
        int dest = graph->edge[e].dest;

        if (solution[var_pos(src, dest, instance)] > 1-TOLERANCE) {
            int x = find(conn_comps, src);
            int y = find(conn_comps, dest);
            if (x != y) {
                union_by_rank(conn_comps, x, y);
                number_of_comps--;
            }
        }
    }

    return number_of_comps;
}

int find(Connected_component conn_comps[], int i) {

    // find root and make root as parent of i (path compression)
    if (conn_comps[i].parent != i) {
        conn_comps[i].parent = find(conn_comps, conn_comps[i].parent);
    }

    return conn_comps[i].parent;
}

void union_by_rank(Connected_component conn_comps[], int x_set, int y_set) {

    //find the root of the set
    int x_root = find(conn_comps, x_set);
    int y_root = find(conn_comps, y_set);

    //attach smaller rank tree under the root of the bigger rank tree (this is the union by rank)
    if (conn_comps[y_root].rank <= conn_comps[x_root].rank) {
        conn_comps[y_root].parent = x_root;
        conn_comps[x_root].rank += conn_comps[y_root].rank;
    } else {
        conn_comps[x_root].parent = y_root;
        conn_comps[y_root].rank += conn_comps[x_root].rank;
    }
}