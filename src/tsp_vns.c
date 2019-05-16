#include "tsp_vns.h"

void tsp_vns_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *solution = calloc(n_edge, sizeof(double));

    get_initial_heuristic_sol(instance, solution, x_pos);

    //int *node_sequence = calloc(n_node, sizeof(int));
    int best_value = 0;
    int max_num_iter = 800;

    two_opt(instance, solution);

    int iter = 0;

    do {
        iter++;
        kick(instance, solution, n_node);
        int new_value = 0;
        two_opt(instance, solution);
        if (new_value < best_value) {
            best_value = new_value;
        }
    } while (iter <= max_num_iter);

    free(solution);
    //free(node_sequence);

}