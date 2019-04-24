#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "input_output.h"
#include "tsp_flow2.h"

int xpos_flow2(int i, int j, Tsp_prob *instance);

int ypos_flow2(int i, int j, Tsp_prob *instance);

int zpos_flow2(int i, int j, Tsp_prob *instance);

void flow2_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *flow2_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = (n_node * n_node); //number of x(i,j) variables
    int n_y_variables = (n_node * n_node); //number of y(i,j) variables
    int n_z_variables = (n_node * n_node); //number of z(i,j) variables
    int n_variables = n_x_variables + n_y_variables + n_z_variables; //total space to allocate for the variable-gurobi map
    DEBUG_PRINT(("Number of nodes: %d\n Number of variables: %d\n", n_node, n_variables));

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    char **variables_names = (char **) calloc((size_t) n_variables, sizeof(char *));
    int optim_status;
    double obj_val;
    int coord;

    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            coord = xpos_flow2(i,j, instance);
            low_bound[coord] = 0.0;
            if (i!=j) {
                up_bound[coord] = 1.0;
                obj_coeff[coord] = distance(i, j, instance);
            } else {
                up_bound[coord] = 0.0;
                obj_coeff[coord] = 0.0;
            }
            var_type[coord] = GRB_BINARY;
            variables_names[coord] = (char *)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "x(%d,%d)", i + 1, j + 1);
        }
    }

    /*y vars*/
    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
            coord = ypos_flow2(i,j, instance);
            low_bound[coord] = 0.0;
            if (i != j) {
                up_bound[coord] = GRB_INFINITY;
            } else {
                up_bound[coord] = 0.0;
            }
            obj_coeff[coord] = 0.0;
            var_type[coord] = GRB_CONTINUOUS;
            variables_names[coord] = (char *)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "y(%d,%d)", i + 1, j + 1);
        }
    }

    /*z vars*/
    for (int i = 0; i < n_node; i++){
        for (int j = 0; j < n_node; j++){
            coord = zpos_flow2(i,j, instance);
            low_bound[coord] = 0.0;
            if (i != j){
                up_bound[coord] = GRB_INFINITY;
            } else {
                up_bound[coord] = 0.0;
            }
            obj_coeff[coord] = 0.0;
            var_type[coord] = GRB_CONTINUOUS;
            variables_names[coord] = (char *)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "z(%d,%d)", i + 1, j + 1);
        }
    }

    error = GRBloadenv(&env, "flow2.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &flow2_model, "flow2", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, flow2_model, error);

    /*Add objective function elements*/
    error = GRBaddvars(flow2_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, flow2_model, error);

    /*Add time limit*/
    add_time_limit(flow2_model, instance);

    /***********
     * CONSTRAINTS
     ***********/

    int constr_var_index[n_node-1];
    double constr_value[n_node-1];
    double rhs = 1.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int index_cur_constr = 0; //count number of constraints, useful when I add lazy constraints

    int l = 0;

    /*Add constraints for indegree*/
    for (int h = 0; h < n_node; h++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (h != j) {
                constr_var_index[l] = xpos_flow2(j, h, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(flow2_model, n_node-1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow2_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_node; h++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (h != j) {
                constr_var_index[l] = xpos_flow2(h, j, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(flow2_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow2_model, error);
        index_cur_constr++;
    }

    /*Add first F2 constraints*/
    int third_constr_var_index[2 * (n_node - 1)];
    double third_constr_coeff[2 * (n_node - 1)];
    rhs = n_node - 1;
    l = 0;

    for (int j = 1; j < n_node; j++) {
        third_constr_var_index[l] = ypos_flow2(0, j, instance);
        third_constr_coeff[l] = 1.0;
        l++;
    }

    for (int j = 1; j < n_node; j++) {
        third_constr_var_index[l] = ypos_flow2(j, 0, instance);
        third_constr_coeff[l] = -1.0;
        l++;
    }

    sprintf(constr_name, "First_F2_constraint_for_i=1");
    error = GRBaddconstr(flow2_model, 2 * (n_node - 1), third_constr_var_index, third_constr_coeff, GRB_EQUAL, rhs, constr_name);
    quit_on_GRB_error(env, flow2_model, error);
    index_cur_constr++;

    /*case i != 1*/
    rhs = -1.0; //errore nel paper se metto 1 modello infeasible

    for (int i = 1; i < n_node; i++) {
        l = 0;
        /*for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = ypos_flow2(i, j, instance);
                third_constr_coeffs[l] = 1.0;
                l++;
                third_constr_var_index[l] = ypos_flow2(j, i, instance);
                third_constr_coeffs[l] = -1.0;
                l++;
            }
        }*/

        for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = ypos_flow2(i, j, instance);
                third_constr_coeff[l] = 1.0;
                l++;
            }
        }

        for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = ypos_flow2(j, i, instance);
                third_constr_coeff[l] = -1.0;
                l++;
            }
        }


        sprintf(constr_name, "First_F2_constraint_for_i=%d", i + 1);
        error = GRBaddconstr(flow2_model, 2 * (n_node - 1), third_constr_var_index, third_constr_coeff, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow2_model, error);
        index_cur_constr++;
    }

    /*Add second F2 constraints*/
    l = 0;
    rhs = 1 - n_node;

    for (int j = 1; j < n_node; j++) {
        third_constr_var_index[l] = zpos_flow2(0, j, instance);
        third_constr_coeff[l] = 1.0;
        l++;
    }

    for (int j = 1; j < n_node; j++) {
        third_constr_var_index[l] = zpos_flow2(j, 0, instance);
        third_constr_coeff[l] = -1.0;
        l++;
    }

    sprintf(constr_name, "Second_F2_constraint_for_i=1");
    error = GRBaddconstr(flow2_model, 2 * (n_node - 1), third_constr_var_index, third_constr_coeff, GRB_EQUAL, rhs, constr_name);
    quit_on_GRB_error(env, flow2_model, error);
    index_cur_constr++;

    /*case i != 1*/
    rhs = 1.0; //errore nel paper se metto -1 modello infeasible

    for (int i = 1; i < n_node; i++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = zpos_flow2(i, j, instance);
                third_constr_coeff[l] = 1.0;
                l++;
            }
        }

        for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = zpos_flow2(j, i, instance);
                third_constr_coeff[l] = -1.0;
                l++;
            }
        }

        sprintf(constr_name, "Second_F2_constraint_for_i=%d", i + 1);
        error = GRBaddconstr(flow2_model, 2 * (n_node - 1), third_constr_var_index, third_constr_coeff, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow2_model, error);
        index_cur_constr++;
    }

    /*Add third F2 constraints*/
    int fourth_constr_var_index[2 * n_node];
    double fourth_constr_coeff[2 * n_node];
    rhs = n_node - 1;

    for (int i = 0; i < n_node; i++) {
        l = 0;
        /*for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = ypos_flow2(i, j, instance);
                third_constr_coeff[l] = 1.0;
                l++;
            }
        }

        for (int j = 0; j < n_node; j++) {
            if (j != i) {
                third_constr_var_index[l] = zpos_flow2(i, j, instance);
                third_constr_coeff[l] = 1.0;
                l++;
            }
        }*/

        for (int j = 0; j < n_node; j++) {
            fourth_constr_var_index[l] = ypos_flow2(i, j, instance);
            fourth_constr_coeff[l] = 1.0;
            l++;
        }

        for (int j = 0; j < n_node; j++) {
            fourth_constr_var_index[l] = zpos_flow2(i, j, instance);
            fourth_constr_coeff[l] = 1.0;
            l++;
        }
        sprintf(constr_name, "Third_F2_constraint_for_i=%d", i + 1);
        //error = GRBaddconstr(flow2_model, 2 * (n_node - 1), third_constr_var_index, third_constr_coeff, GRB_EQUAL, rhs, constr_name);
        error = GRBaddconstr(flow2_model, 2 * n_node, fourth_constr_var_index, fourth_constr_coeff, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow2_model, error);
        index_cur_constr++;
    }

    /*Add fourth F2 constraints*/
    int sixth_constr_var_index[3];
    double sixth_constr_coeffs[3];
    rhs = 0.0;

    for (int i = 0; i < n_node; i++) {
        for (int j = 0; j < n_node; j++) {
                sixth_constr_var_index[0] = xpos_flow2(i, j, instance);
                sixth_constr_coeffs[0] = 1 - n_node;
                sixth_constr_var_index[1] = ypos_flow2(i, j, instance);
                sixth_constr_coeffs[1] = 1.0;
                sixth_constr_var_index[2] = zpos_flow2(i, j, instance);
                sixth_constr_coeffs[2] = 1.0;
                sprintf(constr_name, "Fourth_F2_constraint_for_i=%d", i + 1);
                error = GRBaddconstr(flow2_model, 3, sixth_constr_var_index, sixth_constr_coeffs, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, flow2_model, error);
                index_cur_constr++;
        }

    }

    /*consolidate the model parameters*/
    error = GRBupdatemodel(flow2_model);
    quit_on_GRB_error(env, flow2_model, error);

    /*write model in an  output file*/
    error = GRBwrite(flow2_model, "output_flow2_model.lp");
    quit_on_GRB_error(env, flow2_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(flow2_model);
    quit_on_GRB_error(env, flow2_model, error);

    /* Capture solution information */
    error = GRBgetintattr(flow2_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, flow2_model, error);
    instance->status = optim_status;

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        error = GRBgetdblattr(flow2_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
        quit_on_GRB_error(env, flow2_model, error);
        instance->best_solution = obj_val;
        printf("Optimal objective: %.4e\n", obj_val);

        /*print solution in a file*/
        error = GRBwrite(flow2_model, "flow2_solution.sol");
        quit_on_GRB_error(env, flow2_model, error);
        plot_solution(instance, flow2_model, env, &xpos_flow2);
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

    free_gurobi(env, flow2_model);

}

int xpos_flow2(int i, int j, Tsp_prob *instance) {
    return instance->nnode * i + j;
}

int ypos_flow2(int i, int j, Tsp_prob *instance) {
    return (xpos_flow2(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j;
}

int zpos_flow2(int i, int j, Tsp_prob *instance) {
    return (ypos_flow2(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j;
}