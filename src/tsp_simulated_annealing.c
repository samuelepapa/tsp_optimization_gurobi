#include "tsp_simulated_annealing.h"

double prob_in_range(double min, double max);

void tsp_simulated_annealing_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *incumbent_solution = calloc(n_edge, sizeof(double));
    double *cur_solution = calloc(n_edge, sizeof(double));
    int *cur_node_sequence = calloc(n_node + 1, sizeof(int));
    int *edge_cost = calloc(n_edge, sizeof(int));

    init_genrand64(time(0));

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            edge_cost[x_pos_tsp(i, j, instance)] = distance(i, j, instance);
        }
    }

    int incumbent_value;
    int cur_sol_value = 0;

    get_initial_heuristic_sol(instance, incumbent_solution, x_pos_tsp);

    get_node_path(incumbent_solution, cur_node_sequence, instance);

    incumbent_value = compute_total_distance(instance, cur_node_sequence);

    new_solution(instance, cur_node_sequence, cur_solution);

    printf("First solution value: %d\n", incumbent_value);

    double T = 10000;
    double n = n_node;
    double rho = genrand64_real1() + 1;
    double delta;

    do {

        for (int i = 0; i < n; i++) {

            cur_sol_value = random_two_opt(instance, cur_solution, cur_node_sequence, edge_cost);

            printf("Solution value found: %d\n", cur_sol_value);

            if (cur_sol_value <= incumbent_value) {
                incumbent_value = cur_sol_value;
                new_solution(instance, cur_node_sequence, incumbent_solution);
            } else {
                if (exp(1.0 * (incumbent_value - cur_sol_value) / T) > genrand64_real1()) {
                    incumbent_value = cur_sol_value;
                    new_solution(instance, cur_node_sequence, incumbent_solution);
                }
            }

        }

        delta = (1.0 * cur_sol_value - incumbent_value) / incumbent_value;

        n = rho * n;

        double beta = prob_in_range(0.5, 0.9);

        T = beta * T;

    } while (delta >= 0.5);

    printf("Best heuristic solution value: %d\n", incumbent_value);

    free(incumbent_solution);
    free(cur_solution);
    free(cur_node_sequence);
    free(edge_cost);
}

double prob_in_range(double min, double max) {

    double p_value;

    do {
        p_value = genrand64_real1();

    } while(p_value < min || p_value > max);

    return p_value;
}