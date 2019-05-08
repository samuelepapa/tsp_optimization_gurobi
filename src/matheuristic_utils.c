//
// Created by samuele on 07/05/19.
//

#include "matheuristic_utils.h"

//simply set a dumb cycle
void simple_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));

void naive_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));


void inverse_map_warm_start_type(int model_type, char *target_string) {
    switch (model_type) {
        case 0:
            strcpy(target_string, "simple");
            break;
        case 1:
            strcpy(target_string, "naive");
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
}

void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    switch (instance->warm_start) {
        case 0:
            simple_warm_start(instance, solution, var_pos);
        case 1:
            naive_warm_start(instance, solution, var_pos);
    }
}

void simple_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
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

void naive_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], pred[n_node];

    int count, next_node, cur_node;
    double min_dist = INFINITY;

    next_node = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j =  i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            if (cost[pos] < min_dist && i == 0) {
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

}