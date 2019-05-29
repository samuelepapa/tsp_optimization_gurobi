#include "tsp_simulated_annealing.h"

double prob_in_range(double min, double max);

int random_two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs, int node);

void tsp_simulated_annealing_create(Tsp_prob *instance) {

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    double *best_solution = calloc(n_edge, sizeof(double));
    double *incumbent_solution = calloc(n_edge, sizeof(double));
    int *incumbent_node_sequence = calloc(n_node, sizeof(int));
    int *cur_node_sequence_alloc = calloc(n_node + 1, sizeof(int));
    int *cur_node_sequence = cur_node_sequence_alloc;
    int *edge_cost = calloc(n_edge, sizeof(int));

    init_genrand64(time(0));

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            edge_cost[x_pos_tsp(i, j, instance)] = distance(i, j, instance);
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

    double T = -1 * (0.15 / log(0.30)) * best_value;

    double rho = prob_in_range(1.0, 5.0);
    double n = rho * n_node;
    double delta = 0;
    //double beta = prob_in_range(0.5, 0.99);
    int cur_node = 0;

    double sigma = prob_in_range(0.01, 0.20);
    double std_dev;
    int *std_value = calloc(ceil(n), sizeof(int));
    int n_std_value;
    int l;

    double acceptance_ratio;
    double accept_transition;
    int total_transition;

    int n_not_update_sol = 0;

    int max_not_update = 2 * n;

    int temperature_reduction = 0;

    do {

        l = 0;
        n_std_value = 0;
        accept_transition = 0;
        total_transition = 0;

        for (int j = 0; j < n; j++) {
            cur_node = j % n_node;
            cur_sol_value = random_two_opt(instance, incumbent_solution, cur_node_sequence, edge_cost, cur_node);

            //printf("Solution value found: %d\n", cur_sol_value);

            if (cur_sol_value < incumbent_value) {
                incumbent_value = cur_sol_value;
                copy_node_sequence(incumbent_node_sequence, cur_node_sequence, n_node);
                new_solution(instance, incumbent_node_sequence, incumbent_solution);
                std_value[l] = incumbent_value;
                l++;
                n_std_value++;
                accept_transition++;
                n_not_update_sol = 0;

                if (incumbent_value < best_value) {
                    best_value = incumbent_value;
                    new_solution(instance, incumbent_node_sequence, best_solution);
                }
            } else {
                delta = incumbent_value - cur_sol_value;
                //printf("exp: %g, delta: %g\n", exp(delta / T), delta);
                if (exp(delta / T) > genrand64_real3()) {
                    copy_node_sequence(incumbent_node_sequence, cur_node_sequence, n_node);
                    new_solution(instance, incumbent_node_sequence, incumbent_solution);
                    incumbent_value = cur_sol_value;
                    std_value[l] = incumbent_value;
                    l++;
                    n_std_value++;
                    accept_transition++;
                    n_not_update_sol = 0;
                    printf("Updated despite %g, %d\n", delta, incumbent_value);
                }
            }

            n_not_update_sol++;
            total_transition++;
        }

        printf("HEURSOL %d\n", incumbent_value);

        n_not_update_sol++;

        acceptance_ratio = accept_transition / total_transition;

        std_dev = standard_deviation(std_value, n_std_value);

        T = T / (1 + (log(1 + sigma) * T) / (3 * std_dev));

        temperature_reduction++;

        /*if (exp(delta / T) < 1e-11) {
            printf("Temperature changed.\n");
            T = -1 * (0.15 / log(0.30)) * best_value;
        }*/
        //T = beta * T;
    } while((exp(delta / T) >= 1e-15 || acceptance_ratio >= 0.001 || n_not_update_sol != 10) && n_not_update_sol <= max_not_update); //while (exp(delta / T) > 1e-11);

    printf("Best heuristic solution value: %d\n", best_value);

    plot_solution_fract(instance, best_solution, x_pos_tsp);

    free(incumbent_solution);
    free(incumbent_node_sequence);
    free(best_solution);
    free(std_value);
    //free(cur_node_sequence_alloc);
    free(edge_cost);
}

double prob_in_range(double min, double max) {

    return  min + genrand64_real1() * (max - min);
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
    /* for (int i = 0; i < n_node + 1; i++) {costs
         new_node_sequence[i] = cur_sequence[i];
     }*/
    int best_distance = compute_total_distance(instance, cur_sequence);
    int new_distance = 0;

    //init_genrand64(time(0));
    //int node = floor(genrand64_real1() * (n_node - 1));
    int node_2 = 0;
    while (1) {
        node_2 = floor(genrand64_real3() * (n_node));
        if (node == 0 && node_2 >= 2 && node_2 <= n_node - 2) {
            break;
        }
        if (node == n_node - 1 && node_2 >= 1 && node_2 <= n_node - 3) {
            break;
        }
        if (node_2 != node && node_2 != node - 1 && node_2 != node + 1) {
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
    best_distance = new_distance;

    copy_node_sequence(node_sequence, new_node_sequence, n_node);
    //new_solution(instance, node_sequence, solution);

    free(new_sequence_allocation);
    free(cur_sequence_allocation);

    return best_distance;
}