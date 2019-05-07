//
// Created by samuele on 07/03/19.
//

#include "utils.h"

/**
 * Verify if the identifier of the connected component is present in the list
 * @param comp The pointer to the connected component structure
 * @param curr_comp The identifier of the current connected component
 * @param num_comp The number of node in the connected component
 * @return
 */
int has_component(Connected_comp *comp, int curr_comp, int num_comp);

/**
 * Return the solution value of the x variables
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param xpos The memory location of the x variable
 * @return The value of x after the resolution of the model
 */
double get_solution(GRBenv *env, GRBmodel *model, int xpos);

/**
 * Print the error message associated by error integer value and free the gurobi model and the gurobi environment
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param error Integer error value returned by the gurobi methods
 */
void quit_on_GRB_error(GRBenv *env, GRBmodel *model, int error) {

    if (error) {
        /*error reporting*/
        printf("ERROR: %s\n", GRBgeterrormsg(env));

        /*free model*/
        if (model != NULL) {
            GRBfreemodel(model);
        }
        /*free environment*/
        if (env != NULL) {
            GRBfreeenv(env);
        }
        exit(1);
    }

}

/**
 * Free the gurobi model and the gurobi environment
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 */
void free_gurobi(GRBenv *env, GRBmodel *model) {

    /*free model*/
    GRBfreemodel(model);

    /*free environment*/
    //GRBfreeenv(env);
}

/**
 * Round the distance to the nearest integer value
 * @param x Value of the distance
 * @return Nearest integer value of x
 */

int nint(double x) {
    return (int) (x + 0.5);
}

/**
 * Select the maximum value between two number
 * @param i First number
 * @param j Second number
 * @return Maximum value between i and j
 */
int max(int i, int j) {
    if (i >= j) {
        return i;
    }

    return j;
}

/**
 * Convert coordinate value to geographical latitude or longitude
 * @param coord Coordinate of the node
 * @return Latitude or longitude value in radians
 */
double lat_long(double coord) {

    const double PI = 3.141592;

    double deg = (int) coord;
    double min = coord - deg;
    return PI * (deg + 5.0 * min / 3.0) / 180.0;

}

/**
 * Compute distance between two different nodes
 * @param i_latitude Latitude of the first node
 * @param j_latitude Latitude of the second node
 * @param i_longitude Longitude of the first node
 * @param j_longitude Longitude of the second node
 * @return Distance between two different nodes in kilometers
 */
int dist_from_geo(double i_latitude, double j_latitude, double i_longitude, double j_longitude) {

    const double RRR = 6378.388;

    double q1 = cos(i_longitude - j_longitude);
    double q2 = cos(i_latitude - j_latitude);
    double q3 = cos(i_latitude + j_latitude);
    return (int) (RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

/**
 * Compute the distance between two points in two dimensions with the method described in the weight_type value of instance
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The distance value from i to j
 */
int distance(int i, int j, Tsp_prob *instance) {

    double xd = instance->coord_x[i] - instance->coord_x[j]; //x coordinates difference
    double yd = instance->coord_y[i] - instance->coord_y[j]; //y coordinates difference

    switch (instance->weight_type) {
        case 0: { //Euclidean distances in 2-D
            return nint(sqrt(xd * xd + yd * yd));
        }

        case 1: { //Maximum distances in 2-D
            double x = abs(xd);
            double y = abs(yd);
            return max(nint(x), nint(y));
        }

        case 2: { //Manhattan distances in 2-D
            double x = abs(xd);
            double y = abs(yd);
            return nint(x + y);
        }

        case 3: { //Euclidean distances in 2-D rounded up
            return (int) ceil(sqrt(xd * xd + yd * yd));
        }

        case 4: { //Geographical distances

            double i_latitude = lat_long(instance->coord_x[i]);
            double j_latitude = lat_long(instance->coord_x[j]);

            double i_longitude = lat_long(instance->coord_y[i]);
            double j_longitude = lat_long(instance->coord_y[j]);

            return dist_from_geo(i_latitude, j_latitude, i_longitude, j_longitude);
        }

        case 5: { //Special distance function for problems att48 and att532 (pseudo-Euclidean)
            double rij = sqrt((xd * xd + yd * yd) / 10.0);
            int tij = nint(rij);
            if (tij < rij) {
                return tij + 1;
            } else {
                return tij;
            }
        }

        default: {
            printf("%s\n", "Wrong weight type!");
            exit(1);
        }
    }
}

int map_model_type(char *optarg) {

    DEBUG_PRINT(("options: %s", optarg));
    if (strncmp(optarg, "std", 3) == 0) {
        return 0;
    }

    if (strncmp(optarg, "mtz", 3) == 0) {
        return 1;
    }

    if (strncmp(optarg, "badcompact", 9) == 0) {
        return 2;
    }

    if (strncmp(optarg, "flow1", 5) == 0) {
        return 3;
    }

    if (strncmp(optarg, "flow2", 5) == 0) {
        return 4;
    }

    if (strncmp(optarg, "flow3", 5) == 0) {
        return 5;
    }

    if (strncmp(optarg, "ts1", 3) == 0) {
        return 6;
    }

    if (strncmp(optarg, "ts2", 3) == 0) {
        return 7;
    }

    if (strncmp(optarg, "ts3", 3) == 0) {
        return 8;
    }

    if (strncmp(optarg, "loop", 4) == 0) {
        return 9;
    }

    if (strncmp(optarg, "lazycall", 8) == 0) {
        return 10;
    }

    if (strncmp(optarg, "hardfixing", 12) == 0) {
        return 11;
    }

    if (strncmp(optarg, "usercall", 8) == 0) {
        return 12;
    }
}

void inverse_map_model_type(int model_type, char *target_string) {

    switch (model_type) {
        case 0:
            strcpy(target_string, "std");
            break;
        case 1:
            strcpy(target_string, "mtz");
            break;
        case 2:
            strcpy(target_string, "badcompact");
            break;
        case 3:
            strcpy(target_string, "flow1");
            break;
        case 4:
            strcpy(target_string, "flow2");
            break;
        case 5:
            strcpy(target_string, "flow3");
            break;
        case 6:
            strcpy(target_string, "ts1");
            break;
        case 7:
            strcpy(target_string, "ts2");
            break;
        case 8:
            strcpy(target_string, "ts3");
            break;
        case 9:
            strcpy(target_string, "loop");
            break;
        case 10:
            strcpy(target_string, "lazycall");
            break;
        case 11:
            strcpy(target_string, "hardfixing");
            break;
        case 12:
            strcpy(target_string, "usercall");
            break;
        default:
            strcpy(target_string, "not a model");
    }

}

void find_connected_comps(GRBenv *env, GRBmodel*model, Tsp_prob *instance, Connected_comp *comp,
                          int (*var_pos)(int, int, Tsp_prob *)) {
    int nnode = instance->nnode;

    for (int i = 0; i < nnode; i++) {
        comp->comps[i] = i;
        comp->number_of_items[i] = 1;
    }

    comp->number_of_comps = nnode;

    int c1, c2;

    for (int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            if (get_solution(env, model, var_pos(i, j, instance)) > 1 - TOLERANCE) {
                if (comp->comps[i] != comp->comps[j]) {
                    c1 = comp->comps[i];
                    c2 = comp->comps[j];
                    for (int v = 0; v < nnode; v++) { //update nodes
                        if (comp->comps[v] == c2) {
                            comp->comps[v] = c1;
                            comp->number_of_items[c1]++;
                            comp->number_of_items[c2]--;//TODO this operation can be removed, if everything works correctly, at the end of the cycle it will always be equal to 0.
                        }
                    }
                    comp->number_of_comps--;
                }
            }
        }
    }

    int num_comp = comp->number_of_comps;
    comp->list_of_comps = calloc(num_comp, sizeof(int));

    //int sort_num_items_list[num_comp];

    for (int i = 0; i < num_comp; i++) {
        comp->list_of_comps[i] = -1;
    }
    int t = 0;
    for (int i = 0; i < nnode; i++) {
        int cc = comp->comps[i];
        if (has_component(comp, cc, num_comp) != 0) {
            continue;
        }
        comp->list_of_comps[t] = cc;
        t++;
    }
}

int has_component(Connected_comp *comp, int curr_comp, int num_comp) {

    for (int i = 0; i < num_comp; i++) {
        if (curr_comp == comp->list_of_comps[i]) {
            return 1;
        }
    }

    return 0;
}

double get_solution(GRBenv *env, GRBmodel *model, int xpos) {

    double x_value;
    int error = GRBgetdblattrelement(model, "X", xpos, &x_value);
    quit_on_GRB_error(env, model, error);
    return x_value;
}

void set_seed(GRBmodel *model, Tsp_prob *instance) {
    if (instance->type == 0 && instance->seed > 0 && instance->seed < GRB_MAXINT) {
        int error = GRBsetintparam(GRBgetenv(model), "Seed", instance->seed);
        quit_on_GRB_error(GRBgetenv(model), model, error);
    }
}

void set_time_limit(GRBmodel *model, Tsp_prob *instance) {
    if (instance->type == 0 && instance->time_limit > 0 && instance->time_limit < GRB_INFINITY) {
        int error = GRBsetdblparam(GRBgetenv(model), GRB_DBL_PAR_TIMELIMIT, instance->time_limit);
        quit_on_GRB_error(GRBgetenv(model), model, error);
    }
}

void free_comp_struc(Connected_comp *comp) {
    free(comp->comps);
//    free(comp->list_of_comps);
    free(comp->number_of_items);
    //free(comp->visit_flag);
}

void free_graph(Graph *graph) {
    free(graph->edge);
    free(graph);
}

/**
 * Free memory to avoid leaks, assumes instance is initialized as variable, not dynamically allocated
 * @param instance The pointer to the problem instance
 */
void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    //free(instance->solution);
}

void close_trial(Trial *trial_inst) {
    free(trial_inst->seeds);
    free(trial_inst->filename);
    free(trial_inst->name);
    free(trial_inst->models);
    for (int i = 0; i < trial_inst->n_instances; i++) {
        close_instance(trial_inst->problems[i]);
    }
    free(trial_inst->problems);
    //The filename points to the instances, they are already freed.
    free(trial_inst->instances);
}