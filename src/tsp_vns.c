#include "tsp_vns.h"

void tsp_vns_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *solution = calloc(n_edge, sizeof(double));

    get_initial_heuristic_sol(instance, solution, x_pos);

    two_opt(instance, solution);

    free(solution);

}