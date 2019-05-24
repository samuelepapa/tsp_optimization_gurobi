//
// Created by samuele on 19/05/19.
//

#include "tsp_grasp.h"

typedef struct {
    //the root of the element in the subset tree
    int *i_edge;
    //rank of element in the subset
    int *j_edge;
    //size of the connected component
    int *edge_cost;
} edge_data;

void grasp_randomized_construction(Tsp_prob *instance, double *solution, edge_data *edge);

int grasp_local_search(Tsp_prob *instance, double *solution, edge_data *edge);

//void update_alpha(double *alpha_list, int selected_alpha, int incumbent_value, double *avg_incumb_value);

int x_pos_grasp(int i, int j, Tsp_prob *instance);

double grasp_cost_solution(Tsp_prob *instance, double *solution);

void tsp_grasp_create(Tsp_prob *instance) {
    //the time limit is interpreted as an iteration count in tsp_grasp_create
    int iteration_count = (int) (instance->time_limit);
    int cur_iteration = 0;
    int num_var = (instance->nnode * (instance->nnode - 1)) / 2;

    edge_data edge = {
            .i_edge = calloc(num_var, sizeof(int)),
            .j_edge = calloc(num_var, sizeof(int)),
            .edge_cost = calloc(num_var, sizeof(int))
    };
    double *solution = calloc(num_var, sizeof(double));
    double *cur_solution = calloc(num_var, sizeof(double));
    int incumbent_value = INT_MAX;

    double *tmp_pointer;


    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            edge.edge_cost[x_pos_grasp(i, j, instance)] = distance(i, j, instance);
            edge.i_edge[x_pos_grasp(i, j, instance)] = i;
            edge.j_edge[x_pos_grasp(i, j, instance)] = j;
        }
    }

    /*int alpha_list_size = 5;
    double *alpha_list = calloc(alpha_list_size, sizeof(double));
    double *avg_incumb_value = calloc(alpha_list_size, sizeof(double));

    for (int i = 0; i < alpha_list_size; i++) {
        alpha_list[i] = 1.0 / alpha_list_size;
    }*/

    while (cur_iteration < iteration_count) {

        grasp_randomized_construction(instance, cur_solution, &edge);

        int cur_sol_value = grasp_local_search(instance, cur_solution, &edge);

        /*if (cur_iteration > 1) {
            int selected_sol_pool = (genrand64_int64() / ULLONG_MAX) * (num_pool_element - 1);
            path_relinking(sol_pool[selected_sol_pool], solution);
        }*/

        /*int selected_alpha = (genrand64_int64() / ULLONG_MAX) * (alpha_list_size - 1);
        double alpha = alpha_list[selected_alpha];*/

        printf("GENSOL %d, %d\n", cur_iteration, cur_sol_value);

        if (cur_sol_value < incumbent_value) {
            tmp_pointer = solution;
            solution = cur_solution;
            cur_solution = tmp_pointer;
            incumbent_value = cur_sol_value;
        }

        cur_iteration++;
    }

    printf("Heuristic solution value: %d\n", incumbent_value);

    plot_solution_fract(instance, solution, x_pos_grasp);
    /*free(alpha_list);
   free(avg_incumb_value);*/
    free(edge.edge_cost);
    free(edge.i_edge);
    free(edge.j_edge);
    free(cur_solution);
    free(solution);
}

void grasp_randomized_construction(Tsp_prob *instance, double *solution, edge_data *edge) {

    int n_edge = (instance->nnode * (instance->nnode - 1)) / 2;
    int *not_available_edge = calloc(n_edge, sizeof(int));

    int *rcl = calloc(instance->nnode - 1, sizeof(int));
    int n_element_rcl;
    int c_min;
    int c_max;
    int pos;
    int l;

    init_genrand64(time(0));

    int start_node = floor(genrand64_real1() * (instance->nnode - 1));
    int cur_node = start_node;
    int n_selected_edge = 0;

    for (int i = 0; i < n_edge; i++) {
        solution[i] = 0.0;
    }

    while (n_selected_edge < instance->nnode - 1) {

        l = 0;
        n_element_rcl = 0;

        c_min = INT_MAX;
        c_max = 0;

        /*for (int i = 0; i < n_edge && !not_available_edge[i]; i++) {
            if (edge->edge_cost[i] < c_min) {
                c_min = edge->edge_cost[i];
            }

            if (edge->edge_cost[i] > c_max) {
                c_max = edge->edge_cost[i];
            }
        }*/

        for (int i = 0; i < instance->nnode; i++) {
            if (i != cur_node) {
                pos = x_pos_grasp(cur_node, i, instance);
                if (!not_available_edge[pos]) {
                    if (edge->edge_cost[pos] < c_min) {
                        c_min = edge->edge_cost[pos];
                    }

                    if (edge->edge_cost[pos] > c_max) {
                        c_max = edge->edge_cost[pos];
                    }
                }
            }
        }

        double alpha = genrand64_real1();//prob_in_range(0.5, 0.9);

        double threshold = c_min + alpha * (c_max - c_min);

        for (int j = 0; j < instance->nnode; j++) { //populate the rcl
            if (j != cur_node) {
                pos = x_pos_grasp(cur_node, j, instance);
                if (edge->edge_cost[pos] <= threshold && !not_available_edge[pos]) {
                    rcl[l] = pos;
                    not_available_edge[pos] = 1;
                    n_element_rcl++;
                    l++;
                }
            }

        }

        int choose_pos_rcl = floor(genrand64_real1() * (n_element_rcl - 1));

        int choose_edge = rcl[choose_pos_rcl];

        solution[choose_edge] = 1.0;

        for (int i = 0; i < instance->nnode; i++) {
            if (i != cur_node) {
                pos = x_pos_grasp(cur_node, i, instance);
                not_available_edge[pos] = 1;
            }
        }

        if (edge->i_edge[choose_edge] != cur_node) {
            cur_node = edge->i_edge[choose_edge];
        } else {
            cur_node = edge->j_edge[choose_edge];
        }

        n_selected_edge++;
    }

    solution[x_pos_grasp(start_node, cur_node, instance)] = 1.0;

    free(rcl);
    free(not_available_edge);

    /*struct timespec start, cur;
    double time_elapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    unsigned long long int seed = (unsigned long long int)start.tv_nsec;
    init_genrand64(seed);


    double breathing = 0.5;

    while (time_elapsed < time_limit) {

        clock_gettime(CLOCK_MONOTONIC, &cur);
        time_elapsed = (cur.tv_sec - start.tv_sec);
        printf("GENSOL %g\n", cost_solution(instance, solution, var_pos));
    }*/

}

int grasp_local_search(Tsp_prob *instance, double *solution, edge_data *edge) {

    int *node_sequence = calloc(instance->nnode + 1, sizeof(int));

    get_node_path(solution, node_sequence, instance);

    int best_incumbent = two_opt(instance, solution, node_sequence, edge->edge_cost);

    free(node_sequence);

    return best_incumbent;

}

void grasp_get_solution_pool(Tsp_prob *instance, double **solution_pool, double time_limit, int method) {

}

int x_pos_grasp(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return x_pos_grasp(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

double grasp_cost_solution(Tsp_prob *instance, double *solution) {
    double cost = 0;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            int coord = x_pos_grasp(i, j, instance);
            if (solution[coord] > TOLERANCE) {
                cost += distance(i, j, instance);
            }
        }
    }
    return cost;
}