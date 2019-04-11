//
// Created by samuele on 20/03/19.
//
#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "input_output.h"
#include "tsp_flow1.h"

int xpos_flow1(int i, int j, Tsp_prob *instance);

int ypos_flow1(int i, int j, Tsp_prob *instance);


void flow1_model_create(Tsp_prob *instance){
    GRBenv *env = NULL;
    GRBmodel *flow1_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = (n_node * n_node); //number of x(i,j) variables
    int n_y_variables = (n_node * n_node); //number of z(i,j) variables
    int n_variables = n_x_variables + n_y_variables; //total space to allocate for the variable-gurobi map
    DEBUG_PRINT(("Number of nodes: %d\n Number of variables: %d\n", n_node, n_variables));

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    char **variables_names = (char **) calloc(n_variables, sizeof(char *));
    int optim_status;
    double obj_val;
    double sol;
    int coord = -1;

    int k= 0;

    for(int i = 0; i < n_node; i++){
        for(int j = 0; j < n_node; j++){
            coord = xpos_flow1(i,j, instance);
            low_bound[coord] = 0.0;
            if(i!=j){
                up_bound[coord] = 1.0;
                obj_coeff[coord] = distance(i, j, instance);
            }else{
                up_bound[coord] = 0.0;
                obj_coeff[coord] = 0.0;
            }
            var_type[coord] = GRB_BINARY;
            variables_names[coord] = (char *)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "x(%d,%d)", i+1, j+1 );
        }
    }

    /*y vars*/

    for(int i = 0; i < n_node; i++){
        for(int j = 0; j < n_node; j++){
            coord = ypos_flow1(i,j, instance);
            low_bound[coord] = 0.0;
            if(i!=j){
                up_bound[coord] = GRB_INFINITY;

            }else{
                up_bound[coord] = 0.0;
            }
            obj_coeff[coord] = 0.0;
            var_type[coord] = GRB_CONTINUOUS;
            variables_names[coord] = (char *)calloc(100, sizeof(char));
            sprintf(variables_names[coord], "y(%d,%d)", i+1, j+1 );
        }
    }

    error = GRBloadenv(&env, "flow1.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &flow1_model, "flow1", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, flow1_model, error);

    /*Add objective function elements*/
    error = GRBaddvars(flow1_model, n_variables, 0, NULL, NULL, NULL,
                        obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, flow1_model, error);

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
            if(h != j){
                constr_var_index[l] = xpos_flow1(j, h, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(flow1_model, n_node-1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow1_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_node; h++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if(h != j){
                constr_var_index[l] = xpos_flow1(h, j, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(flow1_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow1_model, error);
        index_cur_constr++;
    }

    //Third set of constraints, max flow if arc is selected
    int third_constr_var_index[2];
    double third_constr_coeffs[2];

    for(int i = 1; i < n_node; i++){
        for(int j = 0; j < n_node; j++){
            if(i != j) {
                third_constr_var_index[0] = ypos_flow1(i, j, instance);
                third_constr_var_index[1] = xpos_flow1(i, j, instance);
                third_constr_coeffs[0] = 1.0;
                third_constr_coeffs[1] = 1 -(n_node);
                sprintf(constr_name, "seventh(%d,%d)", i + 1, j + 1);
                error = GRBaddconstr(flow1_model, 2, third_constr_var_index,third_constr_coeffs , GRB_LESS_EQUAL, 0.0, constr_name);
                quit_on_GRB_error(env, flow1_model, error);
                index_cur_constr++;
            }
        }
    }

    //Forth set of constraints, max inflow into first node is n-1
    int fourth_constr_var_index[n_node - 1];
    double fourth_constr_coeffs[n_node - 1];
    for(int j = 1; j<n_node; j++){
        fourth_constr_var_index[j - 1] = ypos_flow1(j, 0, instance);
        fourth_constr_coeffs[j - 1] = 1.0;
    }
    sprintf(constr_name, "flow_in_1");
    //error = GRBaddconstr(flow1_model, n_node - 1, fourth_constr_var_index,fourth_constr_coeffs , GRB_EQUAL, n_node - 1, constr_name);
    //quit_on_GRB_error(env, flow1_model, error);
    index_cur_constr++;

    //Fifth set of constraints, setting flow exiting a node to 1
    int fifth_constr_var_index[2*(n_node - 1) - 1];
    double fifth_constr_coeffs[2*(n_node - 1) - 1];
    for(int i = 1; i< n_node; i++){
        l = 0;
        for(int j = 0; j < n_node; j++){
            if(i!=j){
                fifth_constr_var_index[l] = ypos_flow1(i,j, instance);
                fifth_constr_coeffs[l] = 1.0;
                l++;
            }
        }
        for(int k = 1; k< n_node; k++){
            if(k!=i){
                fifth_constr_var_index[l] = ypos_flow1(k,i, instance);
                fifth_constr_coeffs[l] = -1.0;
                l++;
            }
        }
        sprintf(constr_name, "flow_out(%d)", i+1);
        error = GRBaddconstr(flow1_model, 2*(n_node - 1) - 1, fifth_constr_var_index,fifth_constr_coeffs , GRB_EQUAL, 1.0, constr_name);
        quit_on_GRB_error(env, flow1_model, error);
        index_cur_constr++;

    }

    /*consolidate the model parameters*/
    error = GRBupdatemodel(flow1_model);
    quit_on_GRB_error(env, flow1_model, error);

    /*write model in an  output file*/
    error = GRBwrite(flow1_model, "output_flow1_model.lp");
    quit_on_GRB_error(env, flow1_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(flow1_model);
    quit_on_GRB_error(env, flow1_model, error);

    /* Capture solution information */
    error = GRBgetintattr(flow1_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, flow1_model, error);

    error = GRBgetdblattr(flow1_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
    quit_on_GRB_error(env, flow1_model, error);

    /*print solution in a file*/
    error = GRBwrite(flow1_model, "flow1_solution.sol");
    quit_on_GRB_error(env, flow1_model, error);

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        printf("Optimal objective: %.4e\n", obj_val);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else {
        printf("Optimization was stopped early\n");
    }

    plot_solution(instance, flow1_model, env, &xpos_flow1);

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);

    free_gurobi(env, flow1_model);

}

int xpos_flow1(int i, int j, Tsp_prob *instance){
    return instance->nnode * i + j;
}

int ypos_flow1(int i, int j, Tsp_prob *instance){
    return (xpos_flow1(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode*i + j;
}

//add_mini_subtours(Tsp_prob *instance)
