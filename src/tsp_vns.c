#include "tsp_vns.h"

void tsp_vns_create(Tsp_prob *instance) {
    srand((unsigned) time(NULL));
    struct timespec start, cur;
    double time_elapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *incumbent_solution = calloc(n_edge, sizeof(double));
    double *cur_solution = calloc(n_edge, sizeof(double));
    int *node_sequence = calloc(n_node + 1, sizeof(int));

    get_initial_heuristic_sol(instance, incumbent_solution, x_pos_metaheuristic);

    int delta = 0;

    two_opt(instance, incumbent_solution, node_sequence);

    int coord;

   int best_value = compute_total_distance(instance, node_sequence);

    int iteration_count = 0;

    double kick_number = 1;
    int drag = 3;
    int max_kick_number = 3;

    do {
        //Shaking
        for (int i = 0; i < (int) kick_number; i++) {
            kick(instance, cur_solution, n_node, incumbent_solution);
        }
        //plot_solution_fract(instance, incumbent_solution, x_pos_metaheuristic);
        int new_value = 0;
        two_opt(instance, cur_solution, node_sequence);
        //plot_solution_fract(instance, incumbent_solution, x_pos_metaheuristic);

        new_value = compute_total_distance(instance, node_sequence);
        delta = new_value - best_value;

        printf("HEURSOL %d, %d\n", iteration_count, new_value);

        if (new_value < best_value) {
            best_value = new_value;
            new_solution(instance, node_sequence, incumbent_solution);
            kick_number = 1;
        } else if (kick_number == max_kick_number) {
            //kick_number = 1;
        } else {
            //increase size of neighborhood
            //kick_number = (kick_number+1.0/drag>max_kick_number)? kick_number: kick_number+1.0/drag;
        }
        iteration_count++;
        clock_gettime(CLOCK_MONOTONIC, &cur);
    } while ((cur.tv_sec - start.tv_sec) < instance->time_limit);

    printf("BEST SOL: %d \n", best_value);

    free(node_sequence);
    //free(incumbent_solution);
}