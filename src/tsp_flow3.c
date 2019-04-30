#include "tsp_flow3.h"

#define LAZY_LEVEL 2
/*LAZY_LEVEL
 * With a value of 1, the constraint can be used to cut off a feasible solution, but it won’t necessarily be pulled in if another lazy constraint also cuts off the solution.
 * With a value of 2, all lazy constraints that are violated by a feasible solution will be pulled into the model.
 * With a value of 3, lazy constraints that cut off the relaxation solution at the root node are also pulled in.
 */

int xpos_flow3(int i, int j, Tsp_prob *instance);

int ypos_flow3(int i, int j, int k, Tsp_prob *instance);


void flow3_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *flow3_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = (n_node * n_node); //number of x(i,j) variables
    int n_y_variables = n_node * n_node * (n_node - 1); //number of y(i,j,t) variables
    int n_variables = n_x_variables + n_y_variables; //total space to allocate for the variable-gurobi map
    printf("Number of nodes: %d\n Number of variables: %d\n", n_node, n_variables);

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
            coord = xpos_flow3(i,j, instance);
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
    for (int k = 1; k < n_node; k++) {
        for (int i = 0; i < n_node; i++) {
            for (int j = 0; j < n_node; j++) {
                coord = ypos_flow3(i, j, k, instance);
                var_type[coord] = GRB_CONTINUOUS;
                low_bound[coord] = 0.0;
                if (i != j) {
                    up_bound[coord] = GRB_INFINITY;
                } else {
                    up_bound[coord] = 0.0;
                }
                obj_coeff[coord] = 0.0;
                variables_names[coord] = (char*)calloc(100, sizeof(char));
                sprintf(variables_names[coord], "y(%d,%d,%d)", i + 1, j + 1, k + 1);
            }
        }

    }
    if (env == NULL) {
        error = GRBloadenv(&env, "flow3.log");
        if (error || env == NULL) {
            printf("Error: couldn't create empty environment.\n");
            exit(1);
        }
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &flow3_model, "flow3", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, flow3_model, error);

    /*Set time limit*/
    //set_time_limit(flow3_model, instance);

    /*Set seed*/
    //set_seed(flow3_model, instance);

    /*Add objective function elements*/
    error = GRBaddvars(flow3_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, flow3_model, error);


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
                constr_var_index[l] = xpos_flow3(j, h, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(flow3_model, n_node-1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_node; h++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            if (h != j) {
                constr_var_index[l] = xpos_flow3(h, j, instance);
                constr_value[l] = 1.0;
                l++;
            }
        }
        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(flow3_model, n_node - 1, constr_var_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add first F3 constraints*/
    int first_constr_var_index[2];
    double first_constr_value[2];
    rhs = 0.0;

    for (int k = 1; k < n_node; k++) {
        for (int i = 0; i < n_node; i++) {
            for (int j = 0; j < n_node; j++) {
                first_constr_var_index[0] = ypos_flow3(i, j, k, instance);
                first_constr_value[0] = 1.0;
                first_constr_var_index[1] = xpos_flow3(i, j, instance);
                first_constr_value[1] = -1.0;
                sprintf(constr_name, "First_F3_constraint_for_i=%d_y=%d_k=%d", i + 1, j + 1, k + 1);
                error = GRBaddconstr(flow3_model, 2, first_constr_var_index, first_constr_value, GRB_LESS_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, flow3_model, error);

                error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
                quit_on_GRB_error(env, flow3_model, error);
                index_cur_constr++;
            }
        }
    }

    /*Add second F3 constraints*/
    int third_constr_var_index[n_node];
    double third_constr_value[n_node];
    rhs = 1.0;

    for (int k = 1; k < n_node; k++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            third_constr_var_index[l] = ypos_flow3(0, j, k, instance);
            third_constr_value[l] = 1.0;
            l++;
        }
        sprintf(constr_name, "Second_F3_constraint_for_k=%d", k + 1);
        error = GRBaddconstr(flow3_model, n_node, third_constr_var_index, third_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);

        error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add third F3 constraints*/
    rhs = 0.0;

    for (int k = 1; k < n_node; k++) {
        l = 0;
        for (int i = 0; i < n_node; i++) {
            third_constr_var_index[l] = ypos_flow3(i, 0, k, instance);
            third_constr_value[l] = 1.0;
            l++;
        }
        sprintf(constr_name, "Third_F3_constraint_for_k=%d", k + 1);
        error = GRBaddconstr(flow3_model, n_node, third_constr_var_index, third_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);

        error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add fourth F3 constraints*/
    rhs = 1.0;

    for (int k = 1; k < n_node; k++) {
        l = 0;
        for (int i = 0; i < n_node; i++) {
            third_constr_var_index[l] = ypos_flow3(i, k, k, instance);
            third_constr_value[l] = 1.0;
            l++;
        }
        sprintf(constr_name, "Fourth_T3_constraint_for_k=%d", k + 1);
        error = GRBaddconstr(flow3_model, n_node, third_constr_var_index, third_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);

        error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add fifth F3 constraints*/
    rhs = 0.0;

    for (int k = 1; k < n_node; k++) {
        l = 0;
        for (int j = 0; j < n_node; j++) {
            third_constr_var_index[l] = ypos_flow3(k, j, k, instance);
            third_constr_value[l] = 1.0;
            l++;
        }
        sprintf(constr_name, "Fifth_T3_constraint_for_k=%d", k + 1);
        error = GRBaddconstr(flow3_model, n_node, third_constr_var_index, third_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, flow3_model, error);

        error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
        quit_on_GRB_error(env, flow3_model, error);
        index_cur_constr++;
    }

    /*Add sixth F3 constraints*/
    int sixth_constr_var_index[2 * (n_node - 1)];
    double sixth_constr_value[2 * (n_node - 1) ];
    rhs = 0.0;

    for (int k = 1; k < n_node; k++) {
        for (int j = 1; j < n_node; j++) {
            if (j != k) {
                l = 0;
                for (int i = 0; i < n_node; i++) {
                    if (i != j) {
                        sixth_constr_var_index[l] = ypos_flow3(i, j, k, instance);
                        sixth_constr_value[l] = 1.0;
                        l++;
                    }

                }

                for (int i = 0; i < n_node; i++) {
                    if (i != j) {
                        sixth_constr_var_index[l] = ypos_flow3(j, i, k, instance);
                        sixth_constr_value[l] = -1.0;
                        l++;
                    }
                }
                sprintf(constr_name, "Sixth_T3_constraint_for_j=%d_k=%d", j + 1, k + 1);
                error = GRBaddconstr(flow3_model, 2 * (n_node - 1), sixth_constr_var_index, sixth_constr_value, GRB_EQUAL, rhs, constr_name);
                quit_on_GRB_error(env, flow3_model, error);

                error = GRBsetintattrelement(flow3_model, "Lazy", index_cur_constr, LAZY_LEVEL);
                quit_on_GRB_error(env, flow3_model, error);
                index_cur_constr++;
            }
        }

    }

    /*consolidate the model parameters*/
    error = GRBupdatemodel(flow3_model);
    quit_on_GRB_error(env, flow3_model, error);

    /*write model in an  output file*/
    error = GRBwrite(flow3_model, "output_flow3_model.lp");
    quit_on_GRB_error(env, flow3_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(flow3_model);
    quit_on_GRB_error(env, flow3_model, error);

    /* Capture solution information */
    error = GRBgetintattr(flow3_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, flow3_model, error);
    instance->status = optim_status;

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        error = GRBgetdblattr(flow3_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
        quit_on_GRB_error(env, flow3_model, error);
        instance->best_solution = obj_val;
        printf("Optimal objective: %.4e\n", obj_val);

        /*print solution in a file*/
        error = GRBwrite(flow3_model, "flow3_solution.sol");
        quit_on_GRB_error(env, flow3_model, error);
        plot_solution(instance, flow3_model, env, &xpos_flow3);
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

    free_gurobi(env, flow3_model);
}

int xpos_flow3(int i, int j, Tsp_prob *instance){
    return instance->nnode * i + j;
}

int ypos_flow3(int i, int j, int k, Tsp_prob *instance){
    return (xpos_flow3(instance->nnode-1, instance->nnode-1, instance)) + 1 + instance->nnode * i + j + (instance->nnode * instance->nnode) * (k - 1);
}

