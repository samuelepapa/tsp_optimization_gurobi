#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "tsp_fischetti.h"
#include "input_output.h"

int xpos(int i, int j, Tsp_prob *instance);
int zpos(int i, int j, Tsp_prob *instance);

void fischetti_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *fischetti_model = NULL;
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
    double sol;

    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;

    //create x variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i+1; j < n_node ; j++) {
            coord = xpos(i, j, instance);
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
    coord = zpos(0, 0, instance);
    var_type[coord] = GRB_BINARY;
    low_bound[coord] = up_bound[coord] = 1;
    obj_coeff[coord] = 0;
    variables_names[coord] = (char *) calloc(100, sizeof(char));
    sprintf(variables_names[coord], "z(%d,%d)", 1, 1);
    printf("z i:%d, j: %d\n", 1, 1);

    //other cases
    for (int i = 1; i < n_node; i++) {
        for (int j = 1; j < n_node ; j++) {
            coord = zpos(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0;
            up_bound[coord] = 1;
            obj_coeff[coord] = 0;
            variables_names[coord] = (char *) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "z(%d,%d)", i + 1, j + 1);
            printf("z i:%d, j: %d\n", i + 1, j + 1);
        }
    }

    /*create environment*/
    error = GRBloadenv(&env, "fischetti.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &fischetti_model, "fischetti", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, fischetti_model, error);

    /*Add objective function elements*/
    error = GRBaddvars(fischetti_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    quit_on_GRB_error(env, fischetti_model, error);

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
                x_constr_index[k] = xpos(i, j, instance);
                x_constr_value[k] = 1.0;
                k++;
            }
        }
        sprintf(constr_name, "deg(%d)", i+1);
        error = GRBaddconstr(fischetti_model, n_node - 1, x_constr_index, x_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, fischetti_model, error);
        index_cur_constr++;
    }

    /*Define and add first and second z(v,h) assignment constraints*/
    int z_constr_index[n_node - 1];
    double z_constr_value[n_node - 1];
    rhs = 1.0;

    for (int v = 0; v < n_node ; v++) {
        k = 0;
        for (int h = 0; h < n_node; h++) {
            if(h != v) {
                z_constr_index[v] = zpos(v, h, instance);
                z_constr_value[v] = 1.0;
                k++;
            }
        }
        sprintf(constr_name, "zac1(%d)", v + 1);
        error = GRBaddconstr(fischetti_model, n_node - 1, z_constr_index, z_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, fischetti_model, error);
        index_cur_constr++;

    }

    for (int h = 0; h < n_node; h++) {
        k = 0;
        for (int v = 0; v < n_node; v++) {
            if(h != v) {
                z_constr_index[v] = zpos(v, h, instance);
                z_constr_value[v] = 1.0;
                k++;
            }
        }
        sprintf(constr_name, "zac2(%d)", h + 1);
        error = GRBaddconstr(fischetti_model, n_node - 1, z_constr_index, z_constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, fischetti_model, error);
        index_cur_constr++;

    }

    /*Define and add incompatibility case constraints*/
    int constr_index[n_node - 1];
    double constr_value[n_node - 1];
    rhs = 2.0;

    for (int i = 1; i < n_node; i++) {
        for (int j = 1; j < n_node; j++) {
            k = 0;
            for (int h = 2; h < n_node; h++) {
                for (int t = 1; t < n_node; t++) {
                    if(t <= h) {
                        constr_index[k] = zpos(i, t, instance);
                        constr_value[k] = 1.0;
                        k++;
                    } else if(t >= h + 2) {
                        constr_index[k] = zpos(j, t, instance);
                        constr_value[k] = 1.0;
                        k++;
                    } else {
                        constr_index[k] = xpos(i, j, instance);
                        constr_value[k] = 1.0;
                        k++;
                    }

                }
            }
            sprintf(constr_name, "zic(%d,%d)", i + 1, j + 1);
            error = GRBaddconstr(fischetti_model, n_node - 1, constr_index, constr_value, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, fischetti_model, error);
            index_cur_constr++;
        }

    }

    /*Define and add additional information constraints*/
    int add_constr_index[n_node - 2];
    double add_constr_value[n_node - 2];
    rhs = 1.0;

    for (int i = 1; i < n_node ; i++) {
        k = 0;
        for (int t = 3; t < n_node - 1; t++) {
            add_constr_index[k] = zpos(i, t, instance);
            add_constr_value[k] = 1.0;
            k++;
        }
        add_constr_index[k] = xpos(i, 1, instance);
        add_constr_value[k] = 1.0;
        sprintf(constr_name, "addc(%d)", i + 1);
        error = GRBaddconstr(fischetti_model, n_node - 2, add_constr_index, add_constr_value, GRB_LESS_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, fischetti_model, error);
        index_cur_constr++;
    }

    /*free memory*/
    free(constr_name);

    for (int i = 0; i < n_variables; i++) {
        free(variables_names[i]);
    }

    free(variables_names);

}

int zpos(int i, int j, Tsp_prob *instance) {
    int latest_x_pos = xpos(instance->nnode - 1, instance->nnode - 1, instance);
    return i * latest_x_pos +j;
}