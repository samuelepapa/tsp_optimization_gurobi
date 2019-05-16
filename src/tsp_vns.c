#include "tsp_vns.h"

void tsp_vns_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *best_solution = calloc(n_edge, sizeof(double));
    int *node_sequence = calloc(n_node + 1, sizeof(int));

    get_initial_heuristic_sol(instance, best_solution, x_pos);

    int delta = 0;

    two_opt(instance, best_solution, node_sequence);

   int best_value = compute_total_distance(instance, node_sequence);


    do {
        kick(instance, best_solution, n_node);
        int new_value = 0;
        two_opt(instance, best_solution, node_sequence);
        new_value = compute_total_distance(instance, node_sequence);
        delta = new_value - best_value;

        if (new_value < best_value) {
            best_value = new_value;
            new_solution(instance, node_sequence, best_solution);
        }

    } while (delta < 0);

    free(node_sequence);
}