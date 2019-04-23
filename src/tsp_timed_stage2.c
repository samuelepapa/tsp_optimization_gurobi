#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "input_output.h"
#include "tsp_timed_stage2.h"


int xpos_ts2(int i, int j, Tsp_prob *instance);

int ypos_ts2(int i, int j, int t, Tsp_prob *instance);

void timed_stage2_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *ts2_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = n_node * n_node; //number of x(i,j) variables
    int n_y_variables = n_node * n_node * n_node; //number of y(i,j,t) variables
    int n_variables = n_x_variables + n_y_variables; //total space to allocate for the variable-gurobi map
    DEBUG_PRINT(("Number of nodes: %d\nNumber of variables: %d\n", n_node, n_variables));

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    char **variables_names = (char **) calloc((size_t) n_variables, sizeof(char *));
    int optim_status;
    double obj_val;
    int coord;

    /*x vars*/
    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            coord = xpos_ts2(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0.0;
            if (i == j) {
                up_bound[coord] = 0.0;
                obj_coeff[coord] = 0.0;
            } else {
                up_bound[coord] = 1.0;
                obj_coeff[coord] = distance(i, j, instance);
            }
            variables_names[coord] = (char*)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "x(%d,%d)", i + 1, j + 1);
        }

    }

    /*y vars*/
    for (int t = 0; t < n_node; t++) {
        for (int i = 0; i < n_node; i++) {
            for (int j = 0; j < n_node; j++) {
                coord = ypos_ts2(i, j, t, instance);
                var_type[coord] = GRB_BINARY;
                low_bound[coord] = 0.0;
                if (i != j) {
                    up_bound[coord] = 1.0;
                } else {
                    up_bound[coord] = 0.0;
                }
                obj_coeff[coord] = 0.0;
                variables_names[coord] = (char*)calloc(100, sizeof(char));
                sprintf(variables_names[coord], "y(%d,%d,%d)", i + 1, j + 1, t + 1);
            }
        }

    }

    error = GRBloadenv(&env, "timed_stage_2.log");
    if (error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    error = GRBnewmodel(env, &ts2_model, "timed_stage_2", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, ts2_model, error);

    /*Add variables to the model*/
    error = GRBaddvars(ts2_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, ts2_model, error);

    /*Constraints*/
    int constr_var_index[n_node - 1];
    double constr_value[n_node - 1];
    double rhs = 1.0;
    char *constr_name = (char*) calloc(100, sizeof(char));
    int index_cur_constr = 0; //count number of constraints, useful when I add lazy constraints

    int q = 0;

    /*Add constraints for indegree*/
    for (int i = 0; i < n_node; i++) {
        q = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j){
                constr_var_index[q] = xpos_ts2(j, i, instance);
                constr_value[q] = 1.0;
                q++;
            }
        }
        sprintf(constr_name, "indeg(%d)", i + 1);
        error = GRBaddconstr(ts2_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int i = 0; i < n_node; i++) {
        q = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j){
                constr_var_index[q] = xpos_ts2(i, j, instance);
                constr_value[q] = 1.0;
                q++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", i + 1);
        error = GRBaddconstr(ts2_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
        index_cur_constr++;
    }

    /*2ND STAGE DEPENDENT CONSTRAINTS*/

    /*first constraints*/
    int  constr_1_index[n_node + 1];
    double constr_1_value[n_node + 1];
    rhs = 0.0;

    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                constr_1_index[0] = xpos_ts2(i, j, instance);
                constr_1_value[0] = 1.0;
                q = 1;
                for (int t = 0; t < n_node; t++) {
                    constr_1_index[q] = ypos_ts2(i, j, t, instance);
                    constr_1_value[q] = -1.0;
                    q++;

                }
                sprintf(constr_name, "First_constraint_(%d,%d)", i + 1, j+1);
                error = GRBaddconstr(ts2_model, n_node + 1, constr_1_index, constr_1_value, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, ts2_model, error);
                index_cur_constr++;
            }
        }

    }

    /*Second constraints*/
    int nnz = n_node * (n_node - 1);
    int constr_2_index[nnz];
    double constr_2_value[nnz];
    rhs = 1.0;

    for (int j = 0; j < n_node; j++) {
        q = 0;
        for (int i = 0; i < n_node; i++) {
            if (i != j) {
                for (int t = 0; t < n_node; t++) {
                    constr_2_index[q] = ypos_ts2(i, j, t, instance);
                    constr_2_value[q] = 1.0;
                    q++;
                }
            }
        }

        sprintf(constr_name, "Second_T2_constraint_for_j=%d", j + 1);
        error = GRBaddconstr(ts2_model, nnz, constr_2_index, constr_2_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
        index_cur_constr++;
    }

    /*Third constraints*/
    for (int i = 0; i < n_node; i++) {
        q = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                for (int t = 0; t < n_node; t++) {
                    constr_2_index[q] = ypos_ts2(i, j, t, instance);
                    constr_2_value[q] = 1.0;
                    q++;
                }
            }
        }

        sprintf(constr_name, "Third_T2_constraint_for_i=%d", i + 1);
        error = GRBaddconstr(ts2_model, nnz, constr_2_index, constr_2_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
        index_cur_constr++;
    }

    /*Fourth constraints*/
    for (int t = 0; t < n_node; t++) {
        q = 0;
        for (int i = 0; i < n_node; i++) {
            for (int j = 0; j < n_node; j++) {
                if (i != j) {
                    constr_2_index[q] = ypos_ts2(i, j, t, instance);
                    constr_2_value[q] = 1.0;
                    q++;
                }
            }
        }

        sprintf(constr_name, "Fourth_T2_constraint_for_t=%d", t + 1);
        error = GRBaddconstr(ts2_model, nnz, constr_2_index, constr_2_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
        index_cur_constr++;
    }


    /*fifth constraints*/
    nnz = (n_node - 1) * (n_node - 1) + n_node * (n_node - 1);
    int constr_3_index[nnz];
    double constr_3_value[nnz];
    rhs = 1.0;

    for (int i = 1; i < n_node; i++) {
        q = 0;
        for (int t = 1; t < n_node; t++) { //t>=2
            for (int j = 0; j < n_node; j++) {
                if (i != j) {
                    constr_3_index[q] = ypos_ts2(i, j, t, instance);
                    constr_3_value[q] = t;
                    q++;
                }
            }
        }

        for (int t = 0; t < n_node; t++) {
            for (int k = 0; k < n_node; k++) {
                if (k != i) {
                    constr_3_index[q] = ypos_ts2(k, i, t, instance);
                    constr_3_value[q] = -1.0 * t;
                    q++;
                }
            }
        }

        sprintf(constr_name, "Fifth_T2_constr_for_i=%d", i + 1);
        error = GRBaddconstr(ts2_model, nnz, constr_3_index, constr_3_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts2_model, error);
    }


    /*consolidate the model parameters*/
    error = GRBupdatemodel(ts2_model);
    quit_on_GRB_error(env, ts2_model, error);

    /*write model in an  output file*/
    error = GRBwrite(ts2_model, "output_timed_stage_2_model.lp");
    quit_on_GRB_error(env, ts2_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(ts2_model);
    quit_on_GRB_error(env, ts2_model, error);

    /* Capture solution information */
    error = GRBgetintattr(ts2_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, ts2_model, error);
    instance->status = optim_status;

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        error = GRBgetdblattr(ts2_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
        quit_on_GRB_error(env, ts2_model, error);
        instance->best_solution = obj_val;
        printf("Optimal objective: %.4e\n", obj_val);

        /*print solution in a file*/
        error = GRBwrite(ts2_model, "timed_stage_2_solution.sol");
        quit_on_GRB_error(env, ts2_model, error);
        plot_solution(instance, ts2_model, env, &xpos_ts2);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else {
        printf("Optimization was stopped early\n");
    }

    plot_solution(instance, ts2_model, env, &xpos_ts2);

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);

    free_gurobi(env, ts2_model);
}

int xpos_ts2(int i, int j, Tsp_prob *instance){
    return instance->nnode * i + j;
}

int ypos_ts2(int i, int j, int t, Tsp_prob *instance){
    return (xpos_ts2(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j + (instance->nnode * instance->nnode) * t;
}