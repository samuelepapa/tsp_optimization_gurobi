//
// Created by samuele on 07/05/19.
//

#include "common.h"
#include "matheuristic_utils.h"

//simply set a dumb cycle
void simple_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *));


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
            simple_warm_start(instance, solution, var_pos);
    }
}

void simple_warm_start(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int nnode = instance->nnode;
    for (int i = 0; i < nnode - 1; i++) {
        solution[var_pos(i, i + 1, instance)] = 1.0;
    }
    solution[var_pos(0, nnode - 1, instance)] = 1.0;
}
