#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "tsp_bad_compact.h"
#include "input_output.h"

int xpos_bad_compact(int i, int j, Tsp_prob *instance);
int zpos_bad_compact(int i, int j, Tsp_prob *instance);

void bad_compact_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *bad_compact_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_x_variables = (int) (0.5 * (n_node * n_node - n_node)); //number of x(i,j) variables (with that formula is always even)
    int n_z_variables = n_node * n_node; //number of z(i,j) variables
    int n_variables = n_x_variables + n_z_variables; //total space to allocate for the variable-gurobi map
    printf("Number of nodes: %d\n Number of variables: %d\n", n_node, n_variables);

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    int optim_status;
    double obj_val;

    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;

    //create x variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i+1; j < n_node ; j++) {
            coord = xpos_bad_compact(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0;
            up_bound[coord] = 1;
            obj_coeff[coord] = distance(i, j, instance);
            variables_names[coord] = (char *) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "x(%d,%d)", i+1, j+1);
            printf("x i:%d, j: %d\n", i + 1, j + 1);
        }
    }

    //add z(v,h) variables
    //case z(1,1)
    /*coord = zpos_bad_compact(1, 1, instance);
    var_type[coord] = GRB_BINARY;

    obj_coeff[coord] = 0;
    variables_names[coord] = (char *) calloc(100, sizeof(char));
    sprintf(variables_names[coord], "z(%d,%d)", 1, 1);
    printf("z i:%d, j: %d\n", 1, 1);*/

    //other cases
    for (int v = 0; v < n_node; v++) {
        for (int h = 0; h < n_node; h++) {
            coord = zpos_bad_compact(v, h, instance);
            var_type[coord] = GRB_BINARY;
            if(v == 0) {
                if (h == 0) {
                    low_bound[coord] = up_bound[coord] = 1;
                } else {
                    low_bound[coord] = up_bound[coord] = 0;
                }
            }else if(h == 0) {
                low_bound[coord] = up_bound[coord] = 0;
            }else{
                low_bound[coord] = 0;
                up_bound[coord] = 1;
            }
            obj_coeff[coord] = 0;
            variables_names[coord] = (char *) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "z(%d,%d)", v + 1, h + 1);
            printf("z v:%d, h: %d\n", v + 1, h + 1);
        }
    }

    /*create environment*/
    error = GRBloadenv(&env, "bad_compact.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &bad_compact_model, "bad_compact", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, bad_compact_model, error);

    /*Set time limit*/
    set_time_limit(bad_compact_model, instance);

    /*Set seed*/
    set_seed(bad_compact_model, instance);

    /*Add objective function elements*/
    error = GRBaddvars(bad_compact_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, bad_compact_model, error);



    /*Define and add constraints to the model*/
    int x_constr_index[n_node - 1];
    double x_constr_value[n_node - 1];
    double rhs = 2.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int k;
    int index_cur_constr = 0; //count number of constraints

    for (int i = 0; i < n_node; i++) {
        k = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                x_constr_index[k] = xpos_bad_compact(i, j, instance);
                x_constr_value[k] = 1.0;
                k++;
            }
        }
        sprintf(constr_name, "deg(%d)", i+1);
        error = GRBaddconstr(bad_compact_model, n_node - 1, x_constr_index, x_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, bad_compact_model, error);
        index_cur_constr++;
    }

    /*Define and add first and second z(v,h) assignment constraints*/
    // variable index
    int z_constr_index[n_node];
    // coefficient
    double z_constr_value[n_node];
    rhs = 1.0;

    for (int v = 1; v < n_node; v++) {
        k = 0;
        for (int h = 0; h < n_node; h++) {
            z_constr_index[k] = zpos_bad_compact(v, h, instance);
            z_constr_value[k] = 1.0;
            k++;
        }
        sprintf(constr_name, "zac1(%d)", v + 1);
        error = GRBaddconstr(bad_compact_model, n_node, z_constr_index, z_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, bad_compact_model, error);
        index_cur_constr++;

    }

    for (int h = 0; h < n_node; h++) {
        k = 0;
        for (int v = 0; v < n_node; v++) {
            z_constr_index[k] = zpos_bad_compact(v, h, instance);
            z_constr_value[k] = 1.0;
            k++;

        }
        sprintf(constr_name, "zac2(%d)", h + 1);
        error = GRBaddconstr(bad_compact_model, n_node, z_constr_index, z_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, bad_compact_model, error);
        index_cur_constr++;

    }

    /*Define and add incompatibility case constraints*/
    int constr_index[n_node - 1];
    double constr_value[n_node - 1];
    rhs = 2.0;

    for (int i = 1; i < n_node; i++) {
        for (int j = 1; j < n_node; j++) {
            if(i != j) {
                for (int h = 2; h < n_node - 1; h++) {
                    k = 0;
                    for (int t = 1; t < n_node; t++) {
                        if(t < h) {
                            constr_index[k] = zpos_bad_compact(i, t, instance);
                            constr_value[k] = 1.0;
                            k++;
                        }

                        if(t >= h + 1) {
                            constr_index[k] = zpos_bad_compact(j, t, instance);
                            constr_value[k] = 1.0;
                            k++;
                        }
                    }
                    //add x(i,j)
                    constr_index[k] = xpos_bad_compact(i, j, instance);
                    constr_value[k] = 1.0;

                    sprintf(constr_name, "zic(%d,%d,%d)", i + 1, j + 1, h);
                    error = GRBaddconstr(bad_compact_model, n_node - 1, constr_index, constr_value, GRB_LESS_EQUAL, rhs, constr_name);
                    quit_on_GRB_error(env, bad_compact_model, error);
                    index_cur_constr++;
                }
            }

        }

    }

    /*Define and add additional information constraints*/
    int add_constr_index[n_node - 2];
    double add_constr_value[n_node - 2];
    rhs = 1.0;

    for (int i = 1; i < n_node ; i++) {
        k = 0;
        for (int t = 2; t < n_node - 1; t++) {
            add_constr_index[k] = zpos_bad_compact(i, t, instance);
            add_constr_value[k] = 1.0;
            k++;
        }
        //add x(i,1)
        add_constr_index[k] = xpos_bad_compact(i, 0, instance);
        add_constr_value[k] = 1.0;

        sprintf(constr_name, "addc(%d)", i + 1);
        error = GRBaddconstr(bad_compact_model, n_node - 2, add_constr_index, add_constr_value, GRB_LESS_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, bad_compact_model, error);
        index_cur_constr++;
    }

    error = GRBupdatemodel(bad_compact_model);
    quit_on_GRB_error(env,bad_compact_model, error);

    /*write model in an  output file*/
    error = GRBwrite(bad_compact_model, "output_bad_compact_model.lp");
    quit_on_GRB_error(env, bad_compact_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(bad_compact_model);
    quit_on_GRB_error(env, bad_compact_model, error);

    /* Capture solution information */
    error = GRBgetintattr(bad_compact_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, bad_compact_model, error);
    instance->status = optim_status;

    /*error = GRBgetdblattrarray(MTZ_model, GRB_DBL_ATTR_X, 0, n_nodes, &sol);
    quit_on_GRB_error(env, MTZ_model, error);*/
    /*print solution in a file*/

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        error = GRBgetdblattr(bad_compact_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
        quit_on_GRB_error(env, bad_compact_model, error);
        instance->best_solution = obj_val;
        printf("Optimal objective: %.4e\n", obj_val);

        error = GRBwrite(bad_compact_model, "solution_bad_compact_model.sol");
        quit_on_GRB_error(env, bad_compact_model, error);

        plot_solution(instance, bad_compact_model, env, &xpos_bad_compact);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else if (optim_status == GRB_TIME_LIMIT) {
        printf("Optimization was stopped early\n");
    }

    /*free memory*/
    free(constr_name);

    for (int i = 0; i < n_variables; i++) {
        free(variables_names[i]);
    }

    free(variables_names);

    free_gurobi(env, bad_compact_model);

}

int xpos_bad_compact(int i, int j, Tsp_prob *instance){
    if(i==j) {
        //printf("Index i=j\n");
        //exit(1);
        return -1;
    }
    if(i>j){
        return xpos_bad_compact(j, i, instance);
    }
    return i*instance->nnode + j - ((i+1)*(i+2))/2;
}

int zpos_bad_compact(int i, int j, Tsp_prob *instance) {
    int latest_x_pos = xpos_bad_compact(instance->nnode - 2, instance->nnode - 1, instance);
    return latest_x_pos + 1 + i * instance->nnode + j;
}