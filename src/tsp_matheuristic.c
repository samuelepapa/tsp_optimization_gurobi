#include "tsp_matheuristic.h"

#define  DEFAULT_TIMELIMIT 120000
int num_it = 0;

void remove_subtour(Tsp_prob *instance, double *solution, int num_conn_comp, Connected_component *conn_comps, int (*var_pos)(int, int, Tsp_prob *));

int set_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

int initialize_heuristic(Tsp_prob *instance);

void tsp_matheuristic_model_create(Tsp_prob* instance) {
    struct timespec start, end;
    double time_elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (instance->time_limit == INFINITY) {
        instance->time_limit = DEFAULT_TIMELIMIT;
    }

    initialize_heuristic(instance);

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;

    printf("timelimit: %g\n", instance->time_limit);
    while (time_elapsed < instance->time_limit) {
        set_constraints(instance, xpos_lazycall);

        tsp_lazycall_model_run(instance);

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;
    }


}

int initialize_heuristic(Tsp_prob *instance) {
    int error;
    instance->best_heur_sol_value = INFINITY;
    instance->heuristic_iteration = -1;
    //first call to the selected model
    switch (instance->black_box) {
        case 9:
            tsp_loop_model_create(instance);
            break;
        case 10: {
            tsp_lazycall_model_generate(instance);
            error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, 1);
            quit_on_GRB_error(instance->env, instance->model, error);

            tsp_lazycall_model_run(instance);

            error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, GRB_MAXINT);
            quit_on_GRB_error(instance->env, instance->model, error);
        }
            break;
        case 12:
            tsp_usercall_model_create(instance);
            break;

    }
    instance->heuristic_iteration = 0;
    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &instance->best_heur_sol_value);
    quit_on_GRB_error(instance->env, instance->model, error);
}

int set_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    double cur_solution;
    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &cur_solution);
    quit_on_GRB_error(instance->env, instance->model, error);
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            error = GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 0.0);
            quit_on_GRB_error(instance->env, instance->model, error);

        }
    }

    double percent_decr = (-(cur_solution - instance->best_heur_sol_value) / instance->best_heur_sol_value) * 100;

    if (percent_decr < 5) {
        if (instance->heuristic_iteration < 10) {
            instance->heuristic_iteration++;
        } else {
            instance->heuristic_iteration = 0;
            if (instance->prob >= 0.1) {
                instance->prob = instance->prob * 0.8;
            }
        }
    }
    if (cur_solution < instance->best_heur_sol_value) {
        instance->best_heur_sol_value = cur_solution;
    }

    double x;

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            error = GRBgetdblattrelement(instance->model, GRB_DBL_ATTR_X, var_pos(i, j, instance), &x);
            quit_on_GRB_error(instance->env, instance->model, error);
            if (x > 1 - TOLERANCE) {
                double r = ((double) rand() / (RAND_MAX));

                if (r <= instance->prob) {
                    GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 1.0);
                }
            }
        }
    }

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);
}

int change_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *), double cur_sol, double *solution,
                       double node_cnt, int num_conn_comp, Connected_component *conn_comps) {

    printf("Enter method change constraints matheuristic.\n");

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 0.0);
        }
    }

    srand((unsigned)node_cnt);

    if (num_conn_comp > 1) {
        remove_subtour(instance, solution, num_conn_comp, conn_comps, var_pos);
    }

    if (cur_sol < instance->best_heur_sol_value) {
        double percent_decr = (- (cur_sol - instance->best_heur_sol_value) / instance->best_heur_sol_value) * 100;

        if (percent_decr < 5) {
            if (num_it < 10) {
                num_it++;
            } else {
                num_it = 0;
                if (instance->prob >= 0.1) {
                    instance->prob = instance->prob * 0.8;
                }

            }

        }

        instance->best_heur_sol_value = cur_sol;
    }

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {

            if (solution[var_pos(i, j, instance)] > 1 - TOLERANCE) {
                double r = ((double) rand() / (RAND_MAX));

                if (r <= instance->prob) {
                    GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 1.0);
                }
            }
        }
    }

}

void remove_subtour(Tsp_prob *instance, double *solution, int num_conn_comp, Connected_component *conn_comps, int (*var_pos)(int, int, Tsp_prob *)) {

    int size = 2 * num_conn_comp;

    int *tour = calloc(size, sizeof(int));

    int *root = calloc(num_conn_comp, sizeof(int));

    get_root(root, num_conn_comp, conn_comps, instance->nnode);

    int h = 0;

    for (int i = 0; i < num_conn_comp; i++) {
        for (int j = 0; j < instance->nnode; j++) {
            if(root[i] != j) {
                if (solution[var_pos(root[i], j, instance)] > 1 - TOLERANCE) {
                    if (find(conn_comps, conn_comps->parent[j]) == root[i]) {
                        if (h == 0) {
                            tour[h] = root[i];
                            tour[size -1] = j;
                            solution[var_pos(root[i], j, instance)] = 0;
                            h++;
                            break;
                        } else {
                            tour[h] = root[i];
                            tour[h + 1] = j;
                            solution[var_pos(root[i], j, instance)] = 0;
                            h += 2;
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < size - 1; i += 2) {
        solution[var_pos(i, i + 1, instance)] = 1;
    }

    free(root);
    free(tour);

    /*for (int i = 0; i < instance->nnode; i++) {
        node_deg[i] = 2;
    }

    int root[num_conn_comp];
    get_root(root, num_conn_comp, conn_comps, instance->nnode);

    for (int i = 0; i < num_conn_comp; i++) {
        if (node_deg[i] == 2) {
            for (int j = i + 1; j < instance->nnode; j++) {
                if (root[i] != find(conn_comps, j)) {
                    continue;
                }
                if (solution[var_pos(i, j, instance)] > 1 - TOLERANCE) {
                    node_deg[root[i]]--;
                    node_deg[j]--;
                    solution[var_pos(i, j, instance)] = 0;
                    break;
                }
            }
        }

    }

    for (int i = 0; i < instance->nnode; i++) {
        if (node_deg[i] < 2) {
            int x_comp = find(conn_comps, i);
            for (int j = i + 1; j < instance->nnode; j++) {
                int y_comp = find(conn_comps, j);
                if (x_comp != y_comp && node_deg[j] < 2) {
                    solution[var_pos(i, j, instance)] = 1;
                    node_deg[i]++;
                    node_deg[j]++;
                    continue;
                }
            }
        }
    }*/
}