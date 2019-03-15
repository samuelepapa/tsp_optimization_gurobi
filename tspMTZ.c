#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "tspMTZ.h"

int ypos(int i, int j, Tsp_prob *instance){

    return i*instance->nnode + j;
}

int upos(int i, Tsp_prob *instance) {
    int latest_y_pos = ypos(instance->nnode - 1, instance->nnode - 1, instance);
    return latest_y_pos + 1 + i;
}

void print_GRB_error(int error, GRBenv *env, char *msg) {
    if (error) {
        printf("%s\n", msg);
        printf("%s\n", GRBgeterrormsg(env));
        exit(1);
    }
}

void quit(GRBenv *env, GRBmodel *model, int error) {

    /*error reporting*/
    if(error) {
        printf("ERROR: %s\n", GRBgeterrormsg(env));
        exit(1);
    }

    /*free model*/
    GRBfreemodel(model);

    /*free environment*/
    GRBfreeenv(env);
}

void preprocessing_MTZ_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *MTZ_model = NULL;
    int error = 0;
    int n_nodes = instance->nnode;
    int n_variables = n_nodes*n_nodes + n_nodes;
    printf("Number of nodes: %d\n Number of variables: %d\n", n_nodes, n_variables);

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    int optim_status;
    double obj_val;

    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;

    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < n_nodes; j++) {
            coord = ypos(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0;
            if (i == j) {
                up_bound[coord] = 0;
            } else {
                up_bound[coord] = 1;
            }
            obj_coeff[coord] = distance(i, j, instance);
            variables_names[coord] = (char*) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "y(%d,%d)", i + 1, j + 1);
            printf("i:%d, j: %d\n", i + 1, j + 1);
        }
    }

    /*Add variables u*/

    for (int i = 0; i < n_nodes; i++) {
        coord = upos(i, instance);
        obj_coeff[coord] = 0.0;
        if (i == 0) {
            low_bound[coord] = 1.0;
            up_bound[coord] = 1.0;
        } else {
            low_bound[coord] = 2.0;
            up_bound[coord] = n_nodes;
        }
        var_type[coord] = GRB_INTEGER;
        variables_names[coord] = calloc(100, sizeof(char));

        sprintf(variables_names[coord], "u(%d)", i + 1);
    }

    /*error = GRBaddvars(MTZ_model, n_nodes, 0, NULL, NULL, NULL, u_obj_val, u_low_bound, u_up_bound, u_type, u_name);
    if (error) {
        quit(env, MTZ_model, error);
    }*/


    /*create environment*/
    error = GRBloadenv(&env, "MTZ.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &MTZ_model, "MTZ", 0, NULL, NULL, NULL, NULL, NULL);

    if(error) {
        quit(env, MTZ_model, error);
    }

    /* Change objective sense to maximization */
    /*error = GRBsetintattr(MTZ_model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    if (error) {
        quit(env, MTZ_model, error);
    }*/

    /*Add objective function elements*/
    error = GRBaddvars(MTZ_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    if (error) {
        quit(env, MTZ_model, error);
    }


    int constr_index[n_nodes];
    double constr_value[n_nodes];
    double rhs = 1.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int indexNextConstraints = 0;

    /*Add constraints for indegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(j, h, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        if (error) {
            quit(env, MTZ_model, error);
        }
        indexNextConstraints++;
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(h, j, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        if (error) {
            quit(env, MTZ_model, error);
        }
        indexNextConstraints++;
    }



    /*Add lazy constraints for y(i,j) + y(j, i) <= 1*/
    /*With a value of 1, the constraint can be used to cut off a feasible solution, but it won’t
necessarily be pulled in if another lazy constraint also cuts off the solution. With a value of 2, all
lazy constraints that are violated by a feasible solution will be pulled into the model. With a value
of 3, lazy constraints that cut off the relaxation solution at the root node are also pulled in.
     */
    int constr_ind[2];
    double constr_val[2] = {1.0, 1.0};


    for (int i = 0; i < n_nodes; i++) {
        for (int j = i+1; j < n_nodes; j++) {
            constr_ind[0] = ypos(i, j, instance);
            constr_ind[1] = ypos(j, i, instance);
            sprintf(constr_name, "lazy_constr_(%d,%d)", i+1, j+1);

            error = GRBaddconstr(MTZ_model, 2, constr_ind, constr_val, GRB_LESS_EQUAL, rhs, constr_name);
            if (error) {
                quit(env, MTZ_model, error);
            }

            error = GRBsetintattrelement(MTZ_model, "Lazy", indexNextConstraints, 2);
            if (error) {
                quit(env, MTZ_model, error);
            }
            error = GRBupdatemodel(MTZ_model);
            if (error) {
                quit(env, MTZ_model, error);
            }
            indexNextConstraints++;
        }
    }

    /*Add constrain y(i, i) = 0 <=> 0 <= y(i,i) <= 0*/

    /*for (int i = 0; i < n_nodes; i++) {
        constr_index[i] = ypos(i, i, instance);
        constr_value[i] = 1.0;
    }

    error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, 0.0, NULL);
    if (error) {
        quit(env, MTZ_model, error);
    }*/



    /*Add MTZ lazy constraints: u(j) >= u(i) +1 - M * (1 - y(i,j))*/
    int M = n_nodes - 1;
    int MTZ_index[3];
    double MTZ_value[3] = {1.0, -1.0, M};


    for (int i = 0; i < n_nodes; i++) {
        for (int j = 1; j < n_nodes; j++) {
            if(i != j) {
                MTZ_index[0] = upos(i, instance);
                MTZ_index[1] = upos(j, instance);
                MTZ_index[2] = ypos(i, j, instance);
                sprintf(constr_name, "MTZ_constr_(%d,%d)", i+1, j+1);

                error = GRBaddconstr(MTZ_model, 3, MTZ_index, MTZ_value, GRB_LESS_EQUAL, M - 1, constr_name);
                if (error) {
                    quit(env, MTZ_model, error);
                }

                error = GRBsetintattrelement(MTZ_model, "Lazy", indexNextConstraints, 2);
                if (error) {
                    quit(env, MTZ_model, error);
                }
                error = GRBupdatemodel(MTZ_model);
                if (error) {
                    quit(env, MTZ_model, error);
                }
                indexNextConstraints++;
            }
        }
    }




    /*consolidate the model parameters*/
    error = GRBupdatemodel(MTZ_model);
    if (error) {
        quit(env, MTZ_model, error);
    }

    /*write model in an  output file*/
    error = GRBwrite(MTZ_model, "output_MTZ_model.lp");
    if (error) {
        quit(env, MTZ_model, error);
    }

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(MTZ_model);
    if (error) {
        quit(env, MTZ_model, error);
    }

    /* Capture solution information */
    error = GRBgetintattr(MTZ_model, GRB_INT_ATTR_STATUS, &optim_status);
    if (error) {
        quit(env, MTZ_model, error);
    }

    error = GRBgetdblattr(MTZ_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
    if (error) {
        quit(env, MTZ_model, error);
    }

     /*error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, 2, sol);
     * if (error) {
        quit(env, MTZ_model, error);
    }*/

    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        printf("Optimal objective: %.4e\n", obj_val);
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
}
