//
// Created by samuele on 07/05/19.
//

#include "matheuristic_utils.h"

/**
 * Simple initial heuristic solution which just links all the nodes in order
 * @param instance the tsp_prob instance
 * @param solution the solution where to write the output
 * @param var_pos function used to map edge notation (i,j) to variable index notation according to the notation used
 */
void simple_initial_heuristic_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));

void naive_initial_heuristic_solution(int start_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *));

void grasp_initial_heuristic_solution(int start_node, double p_greedy, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *));


void inverse_map_warm_start_type(int model_type, char *target_string) {
    switch (model_type) {
        case 0:
            strcpy(target_string, "simple");
            break;
        case 1:
            strcpy(target_string, "naive");
            break;
        case 2:
            strcpy(target_string, "grasp");
            break;
        default:
            strcpy(target_string, "simple");
    }
}

int map_warm_start_type(char *optarg) {
    DEBUG_PRINT(("options: %s", optarg));

    if (strncmp(optarg, "simple", 6) == 0) {
        return 0;
    }

    if (strncmp(optarg, "naive", 5) == 0) {
        return 1;
    }

    if (strncmp(optarg, "grasp", 5) == 0) {
        return 2;
    }
}

void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    switch (instance->warm_start) {
        case 0:
            simple_initial_heuristic_solution(instance, solution, var_pos);
            break;
        case 1:
            naive_initial_heuristic_solution(0, instance, solution, var_pos);
            break;
        case 2:
            grasp_initial_heuristic_solution(0, 0.5, instance, solution, var_pos);
            break;
    }
}

void simple_initial_heuristic_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int nnode = instance->nnode;
    for (int i = 0; i < nnode - 1; i++) {
        solution[var_pos(i, i + 1, instance)] = 1.0;
    }
    solution[var_pos(0, nnode - 1, instance)] = 1.0;
}

void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, 2);
    quit_on_GRB_error(instance->env, instance->model, error);

    //find the warm solution to feed to algorithm
    int nvariables = (int) (0.5 * (instance->nnode * instance->nnode - instance->nnode));
    double *solution = calloc(nvariables, sizeof(double));
    get_initial_heuristic_sol(instance, solution, var_pos);

    error = GRBsetdblattrarray(instance->model, GRB_DBL_ATTR_START, 0, nvariables, solution);
    quit_on_GRB_error(instance->env, instance->model, error);

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);
}

/*void naive_initial_heuristic_solution(int selected_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], pred[n_node];

    int count, next_node, cur_node;
    double min_dist = INFINITY;

    cur_node = selected_node;

    next_node = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j =  i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            if (cost[pos] < min_dist && i == cur_node) {
                min_dist = cost[pos];
                next_node = j;
            }
        }
        visited[i] = 0;
    }

    pred[next_node] = 0;
    visited[next_node] = 1;
    cur_node = next_node;
    next_node = 0;
    count = 1;

    while (count < n_node) {

        min_dist = INFINITY;

        for (int i = 0; i < n_node; i++) {
            if (i != cur_node) {
                int pos = var_pos(cur_node, i, instance);
                if (cost[pos] < min_dist && !visited[i]) {
                    min_dist = cost[pos];
                    next_node = i;
                }
            }
        }

        pred[next_node] = cur_node;
        visited[cur_node] = 1;
        cur_node = next_node;
        count++;
    }

    for (int i = 0; i < n_node; i++) {
        solution[var_pos(pred[i], i, instance)] = 1.0;
    }

}*/

void naive_initial_heuristic_solution(int start_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], next[n_node];

    int count, next_node, cur_node;
    double min_dist = INFINITY;

    cur_node = start_node;

    next_node = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j =  i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            if (cost[pos] < min_dist && i == cur_node) {
                min_dist = cost[pos];
                next_node = j;
            }
        }
        visited[i] = 0;
    }

    next[cur_node] = next_node;
    visited[cur_node] = 1;
    cur_node = next_node;
    next_node = 0;
    count = 1;

    while (count < n_node) {

        min_dist = INFINITY;

        for (int i = 0; i < n_node; i++) {
            if (i != cur_node) {
                int pos = var_pos(cur_node, i, instance);
                if (cost[pos] < min_dist && !visited[i]) {
                    min_dist = cost[pos];
                    next_node = i;
                }
            }
        }

        next[cur_node] = next_node;
        visited[cur_node] = 1;
        cur_node = next_node;
        count++;
    }

    for (int i = 0; i < n_node; i++) {
        if (i != next[i]) {
            solution[var_pos(i, next[i], instance)] = 1.0;
        } else {
            solution[var_pos(i, start_node, instance)] = 1.0;
        }
    }

}

//Find the n_node nodes with minimum distance from curr_node
void get_min_distances(int curr_node, double *dist, int *not_available_distance, int *select_node, int n_select_node, int *visited, int *next, Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {

    double min;
    int num_node = instance->nnode;
    int dist_pos;
    int node;

    for (int k = 0; k < n_select_node; k++) {

        min = INFINITY;

        dist_pos = 0;

        node = 0;

        for (int i = 0; i < num_node; i++) {
            if (i != curr_node && i != next[curr_node]) {
                int pos = var_pos(curr_node, i, instance);
                if (dist[pos] < min && !not_available_distance[pos] && !visited[i]) {
                    min = dist[pos];
                    dist_pos = pos;
                    node = i;
                }
            }
        }

        select_node[k] = node;
        not_available_distance[dist_pos] = 1;
    }

    //reset not available distance
    for (int j = 0; j < n_select_node; j++) {
        int pos = var_pos(curr_node, select_node[j], instance);
        not_available_distance[pos] = 0;
    }
}

void grasp_initial_heuristic_solution(int start_node, double p_greedy, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {

    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], next[n_node];
    int num_selected_node = 3;
    int other_node[num_selected_node];
    int not_available_distance[n_edges];

    int count, next_node, cur_node;
    double min_dist;

    cur_node = start_node;

    for (int i = 0; i < n_node; i++) {
        for (int j =  i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            not_available_distance[pos] = 0;
        }
        visited[i] = 0;
    }

    next_node = 0;
    count = 0;

    while (count < n_node) {

        min_dist = INFINITY;

        double use_greedy = ((double) rand() / (RAND_MAX));

        if (use_greedy <= p_greedy || count > n_node - num_selected_node) { //greedy approach
            for (int i = 0; i < n_node; i++) {
                if (i != cur_node) {
                    int pos = var_pos(cur_node, i, instance);
                    if (cost[pos] < min_dist && !visited[i]) {
                        min_dist = cost[pos];
                        next_node = i;
                    }
                }
            }
        } else { //random approach
            get_min_distances(cur_node, cost, not_available_distance, other_node, num_selected_node, visited, next, instance, var_pos);
            double prob = ((double) rand() / (RAND_MAX));
            double gap = 1.0 / num_selected_node;
            int array_pos = ceil(prob / gap);
            next_node = other_node[array_pos - 1];
        }

        next[cur_node] = next_node;
        visited[cur_node] = 1;
        cur_node = next_node;

        count++;
    }

    for (int i = 0; i < n_node; i++) {
        if (i != next[i]) {
            solution[var_pos(i, next[i], instance)] = 1.0;
        } else {
            solution[var_pos(i, start_node, instance)] = 1.0;
        }
    }
}