#include "tsp_simulated_annealing.h"

double prob_in_range(double min, double max);

int random_two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs, int node);

void tsp_simulated_annealing_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *best_solution = calloc(n_edge, sizeof(double));
    double *incumbent_solution = calloc(n_edge, sizeof(double));
    int *incumbent_node_sequence = calloc(n_node, sizeof(int));
    int *cur_node_sequence = calloc(n_node + 1, sizeof(int));
    int *edge_cost = calloc(n_edge, sizeof(int));

    int mean_edge_cost = 0;

    init_genrand64(time(0));

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            edge_cost[x_pos_tsp(i, j, instance)] = distance(i, j, instance);
            mean_edge_cost += edge_cost[x_pos_tsp(i, j, instance)];
        }
    }

    int incumbent_value;
    int cur_sol_value;
    int best_value;

    get_initial_heuristic_sol(instance, incumbent_solution, x_pos_tsp);

    get_node_path(incumbent_solution, cur_node_sequence, instance);

    best_value = incumbent_value = compute_total_distance(instance, cur_node_sequence);

    new_solution(instance, cur_node_sequence, best_solution);

    printf("First solution value: %d\n", incumbent_value);

    mean_edge_cost = mean_edge_cost / n_edge;

    double T = 100000;
    //double T = mean_edge_cost;

    //double T = (0.15 / (-1.0 * log(0.30))) * incumbent_value;
    double n = n_node;
    double rho = 1.0; //genrand64_real1() + 1;
    double delta = 0;
    double beta = prob_in_range(0.8, 0.99);

    do {

        for (int j = 0; j < n; j++) {
            cur_sol_value = random_two_opt(instance, incumbent_solution, cur_node_sequence, edge_cost, j);

            printf("Solution value found: %d\n", cur_sol_value);

            if (cur_sol_value < incumbent_value) {
                incumbent_value = cur_sol_value;
                copy_node_sequence(incumbent_node_sequence, cur_node_sequence, n_node);
                new_solution(instance, incumbent_node_sequence, incumbent_solution);

                if (incumbent_value < best_value) {
                    best_value = incumbent_value;
                    new_solution(instance, incumbent_node_sequence, best_solution);
                }
            } else {
                delta = incumbent_value - cur_sol_value;
                if (exp(delta / T) > genrand64_real3()) {
                    copy_node_sequence(incumbent_node_sequence, cur_node_sequence, n_node);
                    new_solution(instance, incumbent_node_sequence, incumbent_solution);
                }
            }
        }

        T = beta * T;

    } while (T > 1e-4);

    printf("Best heuristic solution value: %d\n", best_value);

    plot_solution_fract(instance, best_solution, x_pos_tsp);

    free(incumbent_solution);
    free(incumbent_node_sequence);
    free(best_solution);
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

int random_two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs, int node) {
    int n_node = instance->nnode;
    int coord1, coord2, coord3, coord4;
    int *new_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *new_node_sequence = new_sequence_allocation;
    int *cur_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *cur_sequence = cur_sequence_allocation;

    get_node_path(solution, cur_sequence, instance);
    //get_node_path(solution, new_node_sequence, instance);
    for (int i = 0; i < n_node + 1; i++) {
        new_node_sequence[i] = cur_sequence[i];
    }
    int best_distance = compute_total_distance(instance, cur_sequence);
    int new_distance = 0;

    //init_genrand64(time(0));
    //int node = floor(genrand64_real1() * (n_node - 1));
    int node_2 = 0;
    while (1) {
        node_2 = floor(genrand64_real1() * (n_node - 1));

        if (node_2 != node) {
            break;
        }
    }

    int i, k;

    if (node < node_2) {
        i = node;
        k = node_2;
    } else {
        i = node_2;
        k = node;
    }

    coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
    coord2 = x_pos_metaheuristic(cur_sequence[k], cur_sequence[k + 1], instance);
    coord3 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[k], instance);
    coord4 = x_pos_metaheuristic(cur_sequence[i + 1], cur_sequence[k + 1], instance);
    new_distance = best_distance -
                   costs[coord1] -
                   costs[coord2] +
                   costs[coord3] +
                   costs[coord4];

    two_opt_swap(cur_sequence, i, k, n_node, new_node_sequence);
    //copy_node_sequence(cur_sequence, new_node_sequence, n_node + 1);
    cur_sequence = new_node_sequence;
    best_distance = new_distance;

    copy_node_sequence(node_sequence, cur_sequence, n_node + 1);
    //new_solution(instance, node_sequence, solution);

    free(new_sequence_allocation);
    free(cur_sequence_allocation);

    return best_distance;
}