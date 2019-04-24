#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "input_output.h"
#include "tsp_timed_stage1.h"


int xpos_ts1(int i, int j, Tsp_prob *instance);

int ypos_ts1(int i, int j, int t, Tsp_prob *instance);

void timed_stage1_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *ts1_model = NULL;
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
            coord = xpos_ts1(i, j, instance);
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
                coord = ypos_ts1(i, j, t, instance);
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

    error = GRBloadenv(&env, "timed_stage_1.log");
    if (error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    error = GRBnewmodel(env, &ts1_model, "timed_stage_1", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, ts1_model, error);

    /*Add variables to the model*/
    error = GRBaddvars(ts1_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, ts1_model, error);

    /*Add time limit*/
    add_time_limit(ts1_model, instance);

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
                constr_var_index[q] = xpos_ts1(j, i, instance);
                constr_value[q] = 1.0;
                q++;
            }
        }
        sprintf(constr_name, "indeg(%d)", i + 1);
        error = GRBaddconstr(ts1_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts1_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int i = 0; i < n_node; i++) {
        q = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j){
                constr_var_index[q] = xpos_ts1(i, j, instance);
                constr_value[q] = 1.0;
                q++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", i + 1);
        error = GRBaddconstr(ts1_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts1_model, error);
        index_cur_constr++;
    }

    /*1ST STAGE DEPENDENT CONSTRAINTS*/

    /*first constraints*/
    int nnz = n_node * (n_node * (n_node - 1));
    int  constr_1_index[nnz];
    double constr_1_value[nnz];
    rhs = n_node;
    q = 0;

    for (int t = 0; t < n_node; t++) {
        for (int i = 0; i < n_node; i++) {
            for (int j = 0; j < n_node; j++) {
                if (i != j) {
                    constr_1_index[q] = ypos_ts1(i, j, t, instance);
                    constr_1_value[q] = 1.0;
                    q++;
                }
            }
        }
    }

    sprintf(constr_name, "First_T1_constr");
    error = GRBaddconstr(ts1_model, nnz, constr_1_index, constr_1_value, GRB_EQUAL, rhs, constr_name);
    quit_on_GRB_error(env, ts1_model, error);
    index_cur_constr++;

    /*second constraints*/
    nnz = (n_node - 1) * (n_node - 1) + n_node * (n_node - 1);
    int constr_2_index[nnz];
    double constr_2_value[nnz];
    rhs = 1.0;

    for (int i = 1; i < n_node; i++) {
        q = 0;
        for (int t = 1; t < n_node; t++) { //t>=2
            for (int j = 0; j < n_node; j++) {
                if (i != j) {
                    constr_2_index[q] = ypos_ts1(i, j, t, instance);
                    constr_2_value[q] = t;
                    q++;
                }
            }
        }

        for (int t = 0; t < n_node; t++) {
            for (int k = 0; k < n_node; k++) {
                if (k != i) {
                    constr_2_index[q] = ypos_ts1(k, i, t, instance);
                    constr_2_value[q] = -1.0 * t;
                    q++;
                }
            }
        }

        sprintf(constr_name, "Second_T1_constr_for_i=%d", i + 1);
        error = GRBaddconstr(ts1_model, nnz, constr_2_index, constr_2_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts1_model, error);
    }


    int  constr_3_index[n_node + 1];
    double constr_3_value[n_node + 1];
    rhs = 0.0;

    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                constr_3_index[0] = xpos_ts1(i, j, instance);
                constr_3_value[0] = 1.0;
                q = 1;
                for (int t = 0; t < n_node; t++) {
                    constr_3_index[q] = ypos_ts1(i, j, t, instance);
                    constr_3_value[q] = -1.0;
                    q++;

                }
                sprintf(constr_name, "First_constraint_(%d,%d)", i + 1, j+1);
                error = GRBaddconstr(ts1_model, n_node + 1, constr_3_index, constr_3_value, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, ts1_model, error);
                index_cur_constr++;
            }
        }

    }

    /*additional constraints*/

    /*nnz = 1;
    int constr_add_index[nnz];
    double constr_add_value[nnz];
    rhs = 0.0;

    for (int t = 0; t < n_node - 1; t++) { //t != n
        for (int i = 1; i < n_node; i++) {
            constr_add_index[0] = ypos_ts1(i, 1, t, instance);
            constr_add_value[0] = 1.0;
            sprintf(constr_name, "First_additional_constraint_i=%d_j=%d_t=%d", i + 1, 1, t + 1);
            error = GRBaddconstr(ts1_model, nnz, constr_add_index, constr_add_value, GRB_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, ts1_model, error);
        }
    }

    for (int t = 1; t < n_node; t++) { //t != 1
        for (int j = 1; j < n_node; j++) {
            constr_add_index[0] = ypos_ts1(1, j, t, instance);
            constr_add_value[0] = 1.0;
            sprintf(constr_name, "Second_additional_constraint_i=%d_j=%d_t=%d", 1, j+ 1, t + 1);
            error = GRBaddconstr(ts1_model, nnz, constr_add_index, constr_add_value, GRB_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, ts1_model, error);
        }

    }

    for (int i = 1; i < n_node; i++) { //i != 1
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                constr_add_index[0] = ypos_ts1(i, j, 1, instance);
                constr_add_value[0] = 1.0;
                sprintf(constr_name, "Third_additional_constraint_i=%d_j=%d_t=%d", i + 1, j+ 1, 1);
                error = GRBaddconstr(ts1_model, nnz, constr_add_index, constr_add_value, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, ts1_model, error);
            }

        }
    }*/



    /*consolidate the model parameters*/
    error = GRBupdatemodel(ts1_model);
    quit_on_GRB_error(env, ts1_model, error);

    /*write model in an  output file*/
    error = GRBwrite(ts1_model, "output_timed_stage_1_model.lp");
    quit_on_GRB_error(env, ts1_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(ts1_model);
    quit_on_GRB_error(env, ts1_model, error);

    /* Capture solution information */
    error = GRBgetintattr(ts1_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, ts1_model, error);
    instance->status = optim_status;
    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        error = GRBgetdblattr(ts1_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
        quit_on_GRB_error(env, ts1_model, error);
        instance->best_solution = obj_val;
        printf("Optimal objective: %.4e\n", obj_val);

        /*print solution in a file*/
        error = GRBwrite(ts1_model, "timed_stage_1_solution.sol");
        quit_on_GRB_error(env, ts1_model, error);
        plot_solution(instance, ts1_model, env, &xpos_ts1);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else {
        printf("Optimization was stopped early\n");
    }

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);

    free_gurobi(env, ts1_model);
}

int xpos_ts1(int i, int j, Tsp_prob *instance){
    return instance->nnode * i + j;
}

int ypos_ts1(int i, int j, int t, Tsp_prob *instance){
    return (xpos_ts1(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j + (instance->nnode * instance->nnode) * t;
}