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


void inverse_map_warm_start_type(int model_type, char *target_string) {
    switch (model_type) {
        case 0:
            strcpy(target_string, "simple");
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
}

void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    switch (instance->warm_start) {
        case 0:
            simple_initial_heuristic_solution(instance, solution, var_pos);
    }
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

void simple_initial_heuristic_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int nnode = instance->nnode;
    for (int i = 0; i < nnode - 1; i++) {
        solution[var_pos(i, i + 1, instance)] = 1.0;
    }
    solution[var_pos(0, nnode - 1, instance)] = 1.0;
}

void greedy_initial_heuristic_solution(Tsp_prob *instance, double *solution, int(*var_pos)(int, int, Tsp_prob *)) {

}
