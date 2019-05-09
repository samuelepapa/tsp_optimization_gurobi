#include "tsp_local_branching.h"

#define  DEFAULT_TIMELIMIT 120

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


/**
 * Set the local branching constraints to the model
 * @param instance The pointer to the tsp instance
 * @param var_pos The pointer to the mapping method between user variables and Gurobi variables
 * @return An error integer value
 */
int set_local_branch_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

/**
 * Initializes the local branching algorithm by finding solution using warm start + fast run
 * @param instance the tsp prob instance
 * @return error code from Gurobi
 */
int initialize_local_branching(Tsp_prob *instance, double time_limit);

/**
 * Create and add to the model the local branching constraint
 * @param instance The pointer to the tsp instance
 * @param k The number of edges to change
 * @param var_pos The pointer to the mapping method between user variables and Gurobi variables
 */
void generate_local_branching_constraint(Tsp_prob *instance, int k, int (*var_pos)(int, int, Tsp_prob *));

void tsp_local_branching_create(Tsp_prob *instance) {
    struct timespec start, end;
    double time_elapsed;
    const double initial_sol_tl = 10;

    clock_gettime(CLOCK_MONOTONIC, &start);

    double time_limit = instance->time_limit;

    if (time_limit == INFINITY) {
        time_limit = DEFAULT_TIMELIMIT;
    }

    instance->time_limit = max(time_limit / 25, 10);

    initialize_local_branching(instance, initial_sol_tl);

    clock_gettime(CLOCK_MONOTONIC, &end);

    time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;

    printf("timelimit: %g\n", time_limit);

    //Execute at a minimum 25 times, fixed parameter, at least 10 seconds
    GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, instance->time_limit);

    while (time_elapsed < time_limit) {
        switch (instance->black_box) {
            case 9: {
                set_local_branch_constraints(instance, xpos_loop);

                tsp_loop_model_run(instance);
                break;
            }
            case 10: {
                set_local_branch_constraints(instance, xpos_lazycall);
                GRBwrite(instance->model, "output_local_branching_lazycall.lp");
                tsp_lazycall_model_run(instance);
                break;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;
    }

}

int initialize_local_branching(Tsp_prob *instance, double time_limit) {
    int error = 0;
    instance->best_heur_sol_value = INFINITY;
    instance->heuristic_repetition = -1;
    instance->k_value = 5;
    //first call to the selected model
    switch (instance->black_box) {
        case 9: {
            tsp_loop_model_generate(instance);

            set_warm_start(instance, xpos_loop);

            error = GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, time_limit);
            quit_on_GRB_error(instance->env, instance->model, error);

            //Run very fast, if nothing is found keep initial solution set by set_warm_start using work_start method
            tsp_loop_model_run(instance);
        }
            break;
        case 10: {
            tsp_lazycall_model_generate(instance);

            set_warm_start(instance, xpos_lazycall);

            error = GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, time_limit);
            quit_on_GRB_error(instance->env, instance->model, error);

            //Run very fast, if nothing is found keep initial solution set by set_warm_start using work_start method
            tsp_lazycall_model_run(instance);
        }
            break;
        case 12:
            tsp_usercall_model_create(instance);
            break;

    }
    error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, GRB_MAXINT);
    quit_on_GRB_error(instance->env, instance->model, error);

    instance->heuristic_repetition = 0;
    instance->first_heur_iteration = 1;
    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &instance->best_heur_sol_value);
    quit_on_GRB_error(instance->env, instance->model, error);

    return error;
}

int set_local_branch_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {

    int error;
    double cur_solution;

    int num_constraints;

    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &cur_solution);
    quit_on_GRB_error(instance->env, instance->model, error);

    //get the number of constraints in the model
    error = GRBgetintattr(instance->model, GRB_INT_ATTR_NUMCONSTRS, &num_constraints);
    quit_on_GRB_error(instance->env, instance->model, error);

    num_constraints -= 1;

    double percent_decr = (-(cur_solution - instance->best_heur_sol_value) / instance->best_heur_sol_value) * 100;

    if (percent_decr < 5) {
        if (instance->heuristic_repetition < 10) {
            instance->heuristic_repetition++;
        } else {
            instance->heuristic_repetition = 0;
            if (instance->k_value < 20) {
                instance->k_value = instance->k_value * 2;
            }
        }
    }

    if (cur_solution < instance->best_heur_sol_value) {
        instance->best_heur_sol_value = cur_solution;
    }

    if (instance->first_heur_iteration == 0) {
        //remove latest constraint (local branching)
        error = GRBdelconstrs(instance->model, 1, &num_constraints);
        quit_on_GRB_error(instance->env, instance->model, error);
    }

    generate_local_branching_constraint(instance, instance->k_value, var_pos);
    instance->first_heur_iteration = 0;

}

void generate_local_branching_constraint(Tsp_prob *instance, int k, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    int n_node = instance->nnode;
    int nnz = 0;
    int const_index[n_node];
    double constr_value[n_node];
    double rhs = n_node - k;

    char *constr_name = (char *) calloc(100, sizeof(char));

    int l = 0;
    double x_sol_value;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            error = GRBgetdblattrelement(instance->model, GRB_DBL_ATTR_X, var_pos(i, j, instance), &x_sol_value);
            quit_on_GRB_error(instance->env, instance->model, error);

            if (x_sol_value > 1 - TOLERANCE) {
                const_index[l] = var_pos(i, j, instance);
                constr_value[l] = 1.0;
                nnz++;
                l++;
            }
        }
    }

    sprintf(constr_name, "Local_branching_constraint");

    DEBUG_PRINT(("Added constraint %s\n", constr_name));

    error = GRBaddconstr(instance->model, nnz, const_index, constr_value, GRB_GREATER_EQUAL, rhs, constr_name);
    quit_on_GRB_error(instance->env, instance->model, error);

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);

    free(constr_name);
}