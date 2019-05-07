//
// Created by samuele on 06/05/19.
//

#include "tsp_lazycall.h"
#include "tsp_usercall.h"
#include "tsp_hardfixing.h"
#include "concorde.h"


#define TOLERANCE 10E-4
#define MAX_VARNAME_SIZE 128

int xpos_usercall(int i, int j, Tsp_prob *instance);

void add_flow_cut(void *cbdata, struct callback_data *user_cbdata, int *left_nodes, int left_nodecount, int node);


int __stdcall usercallback(GRBmodel *model, void *cbdata, int where, void *usrdata) {
    struct callback_data *mydata = (struct callback_data *) usrdata;

    if (where == GRB_CB_MIPSOL) {
        double *solution = calloc(mydata->nvars, sizeof(double));
        GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, solution);

        double node_cnt = -1;
        GRBcbget(cbdata, where, GRB_CB_MIPSOL_NODCNT, &node_cnt);

        double sol_value;
        GRBcbget(cbdata, where, GRB_CB_MIPSOL_OBJ, &sol_value);

        struct timespec start, end;
        double time_elapsed;

        clock_gettime(CLOCK_MONOTONIC, &start);

        int number_of_comps = union_find(mydata->graph, solution, &xpos_usercall, mydata->instance, mydata->conn_comps);

        clock_gettime(CLOCK_MONOTONIC, &end);

        time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;

        printf("\nThe current solution has: %d connected components, found in %g seconds \n", number_of_comps,
               time_elapsed);

        if (number_of_comps >= 2) {
            add_lazy_sec(cbdata, mydata, mydata->conn_comps, number_of_comps, (int) node_cnt);
        }

        free(solution);
    }

    if (where == GRB_CB_MIPNODE) {
        //the probability that the calculations to find the connected components and min flow when a solution to
        //the relaxation is found is 1/random_divider
        const int random_divider = 10;
        //these nodes is where the calculations are always run
        const int initial_nodes = 10;
        int number = rand();
        double node_cnt = -1;
        GRBcbget(cbdata, where, GRB_CB_MIPNODE_NODCNT, &node_cnt);
        //do this always at the root and first 10 nodes, then 10% of the times
        if (((int) node_cnt <= initial_nodes) || number < RAND_MAX / random_divider) {
            double *solution = calloc(mydata->nvars, sizeof(double));
            GRBcbget(cbdata, where, GRB_CB_MIPNODE_REL, solution);


            int number_of_comps = union_find(mydata->graph, solution, &xpos_usercall, mydata->instance,
                                             mydata->conn_comps);
            //plot_solution_fract(mydata->instance, solution, &xpos_usercall);
            if (number_of_comps > 1) {
                add_lazy_sec(cbdata, mydata, mydata->conn_comps, number_of_comps, (int) node_cnt);
            } else {
                Tsp_prob *instance = mydata->instance;
                /*  int CCcut_mincut (int ncount, int ecount, int *elist, double *dlen,     */
                /*      double *cutval, int **cut, int *cutcount)                           */
                /*     COMPUTES the global minimum cut in an undirected graph.              */
                /*      -ncount is the number of nodes in the graph.                        */
                /*      -ecount is the number of edges in the graph.                        */
                /*      -elist is the list of edges in end0 end1 format                     */
                /*      -dlen is a list of the edge capacities                              */
                /*      -cutval returns the capacity of the mincut (it can be NULL).        */
                /*      -cut will return the indices of the nodes in the minimum cut;       */
                /*       this variable can be passed in as NULL, otherwise it will be       */
                /*       an allocated to an array of the appropriate length.                */
                /*      -cutcount will return the number of nodes in the minimum cut if     */
                /*       cut is not NULL (if cut is NULL, then cutcount can be NULL).       */
                /*    NOTES: This function assumes graph is connected. Paths of 1's are     */
                /*     are shrunk - this is valid for the tsp, but not in general.          */
                int ncount = instance->nnode;
                int ecount = (int) (0.5 * (ncount * ncount - ncount));
                int failure = 0;
                double cutval = 0;
                int *cut;
                int cutcount = 0;

                failure = CCcut_mincut(ncount, ecount, mydata->edge_list, solution, &cutval, &cut, &cutcount);

                if (cutval < 2) {
                    if (!failure) {
                        add_flow_cut(cbdata, mydata, cut, cutcount, (int) node_cnt);

                    }
                }
            }
        }
    }
    return 0;
}

void tsp_usercall_model_create(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *lazycall_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_variables = (int) (0.5 * (n_node * n_node - n_node)); //this number is always even
    DEBUG_PRINT(("%d", n_variables));
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char **variables_names = (char **) calloc((size_t) n_variables, sizeof(char *));
    int size_variable_names = 0;

    int *edge_list = calloc(2 * n_variables, sizeof(int));

    struct callback_data user_cbdata;

    user_cbdata.instance = instance;
    user_cbdata.lazyconst_id = 0;
    user_cbdata.userconst_id = 0;

    //create graph
    user_cbdata.graph = malloc(sizeof(Graph));
    create_graph_u_f(instance, user_cbdata.graph);

    Connected_component conn_comp = {
            .parent = calloc(n_node, sizeof(int)),
            .rank = calloc(n_node, sizeof(int)),
            .size = calloc(n_node, sizeof(int))
    };

    //Set connected components structure
    user_cbdata.conn_comps = &conn_comp;

    int coord = 0;
    //Create variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            coord = xpos_usercall(i, j, instance);
            upper_bounds[coord] = 1.0;
            lower_bounds[coord] = 0.0;
            variable_type[coord] = GRB_BINARY;
            objective_coeffs[coord] = distance(i, j, instance);
            variables_names[coord] = (char *) calloc(MAX_VARNAME_SIZE, sizeof(char)); //TODO dealloc after
            sprintf(variables_names[coord], "x(%d,%d)", i + 1, j + 1);
            DEBUG_PRINT(("i: %d, ; j: %d\n", i + 1, j + 1));
            size_variable_names++;

            edge_list[2 * coord] = i;
            edge_list[2 * coord + 1] = j;
        }
    }

    //Set edge list for Concorde
    user_cbdata.edge_list = edge_list;

    if (env == NULL) {
        DEBUG_PRINT(("Env was NULL, creating it.\n"));
        //Env creation and starting
        error = GRBloadenv(&env, NULL);
        if (error || env == NULL) {
            printf("Error: couldn't create empty environment.\n");
            exit(1);
        }
    }

    error = GRBnewmodel(env, &lazycall_model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, lazycall_model, error);

    if (instance->model_type == 11) {
        instance->model = lazycall_model;
    }

    error = GRBsetintparam(GRBgetenv(lazycall_model), GRB_INT_PAR_LAZYCONSTRAINTS, 1);
    //error = GRBsetintparam(env, GRB_INT_PAR_LAZYCONSTRAINTS, 1);
    quit_on_GRB_error(env, lazycall_model, error);

    error = GRBsetintparam(GRBgetenv(lazycall_model), GRB_INT_PAR_PRECRUSH, 1);
    quit_on_GRB_error(env, lazycall_model, error);

    /*Set time limit*/
    //set_time_limit(lazycall_model, instance);

    /*Set seed*/
    //set_seed(lazycall_model, instance);

    //Add variables to lazycall_model
    error = GRBaddvars(lazycall_model, n_variables, 0, NULL, NULL, NULL,
                       objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    quit_on_GRB_error(env, lazycall_model, error);


    //Define constraints
    int constr_index[n_node - 1];
    double constr_value[n_node - 1];
    int k = 0;
    double rhs = 2.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int index_cur_constr = 0; //count number of constraints, useful when I add lazy constraints

    for (int i = 0; i < n_node; i++) {
        k = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                constr_index[k] = xpos_usercall(i, j, instance);
                constr_value[k] = 1.0;
                k = k + 1;
            }
        }
        sprintf(constr_name, "deg(%d)", i + 1);
        error = GRBaddconstr(lazycall_model, n_node - 1, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, lazycall_model, error);
        index_cur_constr++;
    }

    int done = 0;
    double solution;
    //Update the model using new constraints
    error = GRBupdatemodel(lazycall_model);
    quit_on_GRB_error(env, lazycall_model, error);

    //get number of vars
    error = GRBgetintattr(lazycall_model, GRB_INT_ATTR_NUMVARS, &user_cbdata.nvars);
    quit_on_GRB_error(env, lazycall_model, error);


    error = GRBsetcallbackfunc(lazycall_model, usercallback, (void *) &user_cbdata);
        quit_on_GRB_error(env, lazycall_model, error);


    error = GRBoptimize(lazycall_model);
    quit_on_GRB_error(env, lazycall_model, error);

    int optim_status = 0;
    error = GRBgetintattr(lazycall_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, lazycall_model, error);
    instance->status = optim_status;


    plot_solution(instance, lazycall_model, env, &xpos_usercall);


    //Freeing memory
    free(constr_name);

    for (int i = 0; i < size_variable_names; i++) {
        free(variables_names[i]);
    }

    free_comp(&conn_comp);

    free(variables_names);

    free_graph(user_cbdata.graph);

    free_gurobi(env, lazycall_model);

}

int xpos_usercall(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return xpos_usercall(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

void add_flow_cut(void *cbdata, struct callback_data *user_cbdata, int *left_nodes, int left_nodecount, int node) {
    int nnode = user_cbdata->instance->nnode;
    int right_nodecount = nnode - left_nodecount;
    int *right_nodes = calloc(right_nodecount, sizeof(int));
    int *node_bitmap = calloc(nnode, sizeof(int));

    //define complementary set
    for (int i = 0; i < left_nodecount; i++) {
        node_bitmap[left_nodes[i]] = 1;
    }
    //find nodes in complementary set
    int t = 0;
    for (int i = 0; i < nnode; i++) {
        if (!node_bitmap[i]) {
            right_nodes[t++] = i;
        }
    }

    int tot_item = left_nodecount * right_nodecount;
    double rhs = 2.0;
    int *constr_index = malloc(sizeof(int) * tot_item);
    double *constr_value = malloc(sizeof(double) * tot_item);
    char *constr_name = malloc(sizeof(char) * 100);
    int error;
    t = 0;
    for (int i = 0; i < left_nodecount; i++) {
        for (int j = 0; j < right_nodecount; j++) {
            constr_index[t] = xpos_usercall(left_nodes[i], right_nodes[j], user_cbdata->instance);
            constr_value[t] = 1.0;
            t++;
        }
    }
    sprintf(constr_name, "add_constr_usercut_%d_%d", node, user_cbdata->userconst_id);
    //printf("Adding constraint: add_constr_usercut_%d_%d\n", node, user_cbdata->userconst_id);
    error = GRBcbcut(cbdata, t, constr_index, constr_value, GRB_GREATER_EQUAL, rhs);
    if (error) {
        printf("Error on user callback adding usercut constraints, code: %d \n", error);
    }
    user_cbdata->userconst_id++;


    free(constr_index);
    free(constr_value);
    free(constr_name);
    free(right_nodes);
    free(node_bitmap);
}