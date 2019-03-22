#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "input_output.h"
#include "tsp_timed_stage3.h"

int xpos_ts3(int i, int j, Tsp_prob *instance);

int ypos_ts3(int i, int j, int t, Tsp_prob *instance);

void timed_stage3_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *ts3_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = n_node * n_node; //number of x(i,j) variables
    int n_y_variables = n_node * n_node * n_node; //number of y(i,j,t) variables
    int n_variables = n_x_variables + n_y_variables; //total space to allocate for the variable-gurobi map
    printf("Number of nodes: %d\nNumber of variables: %d\n", n_node, n_variables);

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    char **variables_names = (char **) calloc(n_variables, sizeof(char *));
    int optim_status;
    double obj_val;
    double sol;
    int coord = -1;

    /*x vars*/
    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            coord = xpos_ts3(i, j, instance);
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
                coord = ypos_ts3(i, j, t, instance);
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

    error = GRBloadenv(&env, "timed_stage_3.log");
    if (error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    error = GRBnewmodel(env, &ts3_model, "timed_stage_3", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, ts3_model, error);

    /*Add variables to the model*/
    error = GRBaddvars(ts3_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, ts3_model, error);

    /*Constraints*/
    int constr_var_index[n_node - 1];
    double constr_value[n_node - 1];
    double rhs = 1.0;
    char *constr_name = (char*) calloc(100, sizeof(char));
    int index_cur_constr = 0; //count number of constraints, useful when I add lazy constraints

    int l = 0;

    /*Add constraints for indegree*/
    for (int i = 0; i < n_node; i++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j){
                constr_var_index[l] = xpos_ts3(j, i, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "indeg(%d)", i + 1);
        error = GRBaddconstr(ts3_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts3_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int i = 0; i < n_node; i++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j){
                constr_var_index[l] = xpos_ts3(i, j, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", i + 1);
        error = GRBaddconstr(ts3_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, ts3_model, error);
        index_cur_constr++;
    }

    /*3RD STAGE DEPENDENT CONSTRAINTS*/
    int  constr_1_index[n_node + 1];
    double constr_1_value[n_node + 1];
    rhs = 0.0;

    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                l = 0;
                for (int t = 0; t < n_node; t++) {
                    constr_1_index[l] = ypos_ts3(i, j, t, instance);
                    constr_1_value[l] = -1.0;
                    l++;

                }
                constr_1_index[l] = xpos_ts3(i, j, instance);
                constr_1_value[l] = 1.0;
                sprintf(constr_name, "First_constraint_(%d,%d)", i + 1, j+1);
                error = GRBaddconstr(ts3_model, n_node + 1, constr_1_index, constr_1_value, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, ts3_model, error);
                index_cur_constr++;
            }
        }

    }

    rhs = 1.0;
    for (int j = 0; j < n_node; j++) {
        constr_var_index[j - 1] = ypos_ts3(0, j, 0, instance);
        constr_value[j - 1] = 1.0;
    }
    sprintf(constr_name, "Second_T3_constraint");
    error = GRBaddconstr(ts3_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
    quit_on_GRB_error(env, ts3_model, error);
    index_cur_constr++;

    for (int i = 1; i < n_node; i++) {
        constr_var_index[i - 1] = ypos_ts3(i, 0, n_node - 1, instance);
        constr_value[i - 1] = 1.0;
    }
    sprintf(constr_name, "Third_T3_constraint");
    error = GRBaddconstr(ts3_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
    quit_on_GRB_error(env, ts3_model, error);
    index_cur_constr++;

    int constr_2_index[2 * n_node];
    double constr_2_value[2* n_node];
    rhs = 0.0;

    for (int i = 1; i < n_node; i++) {
        for (int t = 1; t < n_node; t++) {
            l = 0;
            for (int j = 0; j < n_node; j++) {
                constr_2_index[l] = ypos_ts3(i, j, t, instance);
                constr_2_value[l] = 1.0;
                constr_2_index[l + n_node] = ypos_ts3(j, i, t - 1, instance);
                constr_2_value[l + n_node] = -1.0;
                l++;
            }

            sprintf(constr_name, "Fourth_T3_constraint(%d,%d)", i + 1, t + 1);
            error = GRBaddconstr(ts3_model, 2 * n_node, constr_2_index, constr_2_value, GRB_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, ts3_model, error);
            index_cur_constr++;
        }

    }

    /*consolidate the model parameters*/
    error = GRBupdatemodel(ts3_model);
    quit_on_GRB_error(env, ts3_model, error);

    /*write model in an  output file*/
    error = GRBwrite(ts3_model, "output_timed_stage_3_model.lp");
    quit_on_GRB_error(env, ts3_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(ts3_model);
    quit_on_GRB_error(env, ts3_model, error);

    /* Capture solution information */
    error = GRBgetintattr(ts3_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, ts3_model, error);

    error = GRBgetdblattr(ts3_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
    quit_on_GRB_error(env, ts3_model, error);

    /*print solution in a file*/
    error = GRBwrite(ts3_model, "timed_stage_3_solution.sol");
    quit_on_GRB_error(env, ts3_model, error);

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        printf("Optimal objective: %.4e\n", obj_val);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else {
        printf("Optimization was stopped early\n");
    }

    plot_solution(instance, ts3_model, env, &xpos_ts3);

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);

}

int xpos_ts3(int i, int j, Tsp_prob *instance){
    return instance->nnode * i + j;
}

int ypos_ts3(int i, int j, int t, Tsp_prob *instance){
    return (xpos_ts3(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j + (instance->nnode * instance->nnode) * t;
}