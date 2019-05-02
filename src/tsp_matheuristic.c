#include "tsp_matheuristic.h"

int num_it = 0;
int selected_prob = 0;


void tsp_matheuristic_model_create(Tsp_prob* instance) {
    double p[3] = {0.9, 0.5, 0.2};
    instance->prob = p;
    instance->best_heur_sol_value = INFINITY;
    //first call to the selected model
    tsp_lazycall_model_create(instance);
}

int change_constraints(Tsp_prob* instance, int (*var_pos)(int, int, Tsp_prob *), double cur_sol, double *solution, double node_cnt) {

    printf("Enter method change constraints matheuristic.\n");

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 0.0);
        }
    }

    if (cur_sol < instance->best_heur_sol_value) {
        double percent_decr = (- (cur_sol - instance->best_heur_sol_value) / instance->best_heur_sol_value) * 100;

        if (percent_decr < 10) {
            if (num_it < 10) {
                num_it++;
            } else {
                num_it = 0;
                if (selected_prob < 3) {
                    selected_prob++;
                }
            }

        }

        srand((unsigned)node_cnt);
        instance->best_heur_sol_value = cur_sol;

        for (int i = 0; i < instance->nnode; i++) {
            for (int j = i + 1; j < instance->nnode; j++) {

                if (solution[var_pos(i, j, instance)] > 1 - TOLERANCE) {
                    double r = ((double) rand() / (RAND_MAX));

                    if (r <= instance->prob[selected_prob]) {
                        GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 1.0);
                    }
                }
            }
        }
    }

}