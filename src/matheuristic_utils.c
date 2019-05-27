//
// Created by samuele on 07/05/19.
//

#include <input_output.h>
#include "matheuristic_utils.h"


/**
 * Populates an array of grasped_nodes nodes with the first nodes when looking at the cost of going from first_node to
 * the node being evaluated, back to secondo node: c(1,j)+c(j,2). The grasped_nodes js such that this is done
 * with minimal cost are added to the min_list in ascending order.
 * @param first_node the first node where to start
 * @param second_node the second node where to arrive, first_node and second_node define the arc selected in extra
 * mileage
 * @param costs the array of costs of all the arcs
 * @param min_list the list where to save the results
 * @param grasped_nodes the number of nodes to select
 * @param visited the nodes that have been already visited
 * @param instance the Tsp_prob instance
 * @param var_pos the function to calculate the position of the variables
 */
void
extra_mileage_get_mins(int first_node, int second_node, double *costs, int *min_list, int grasped_nodes, char *visited,
                       Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

/**
 * Converts an extra mileage style cycle to a solution
 * @param n_node the number of nodes in the cycle
 * @param cur_cycle the current cycle, list of nodes in the cycle
 * @param solution the solution where to write the edges that need to be set to 1.0
 * @param instance the Tsp_prob instance
 * @param var_pos the function used to convert (i,j) in the variable position in solution
 */
void extra_mileage_cycle_to_solution(int n_node, int *cur_cycle, double *solution, Tsp_prob *instance,
                                     int (*var_pos)(int, int, Tsp_prob *));

/**
 * Inserts nodes based on cost, ascending order
 * @param cost the cost to add
 * @param node_pos the node index
 * @param min_list the list where to add
 * @param min_costs_list the list of costs already added
 * @param list_size the size of the list up to now
 */
void extra_mileage_insert_sorted_nodes(double cost, int node_pos, int *min_list, double *min_costs_list, int list_size);

void inverse_map_warm_start_type(int model_type, char *target_string) {
    switch (model_type) {
        case 0:
            strcpy(target_string, "simple");
            break;
        case 1:
            strcpy(target_string, "naive");
            break;
        case 2:
            strcpy(target_string, "grasp");
            break;
        case 3:
            strcpy(target_string, "mileage_grasp");
        default:
            strcpy(target_string, "simple");
    }
}

int map_warm_start_type(char *optarg) {
    DEBUG_PRINT(("options: %s", optarg));

    if (strncmp(optarg, "simple", 6) == 0) {
        return 0;
    }

    if (strncmp(optarg, "naive", 5) == 0) {
        return 1;
    }

    if (strncmp(optarg, "grasp", 5) == 0) {
        return 2;
    }

    if (strncmp(optarg, "mileage_grasp", 13) == 0) {
        return 3;
    }
}

void get_initial_heuristic_sol(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    srand(time(NULL));
    switch (instance->warm_start) {
        case 0:
            simple_initial_heuristic_solution(instance, solution, var_pos);
            break;
        case 1:
            naive_initial_heuristic_solution(0, instance, solution, var_pos);
            break;
        case 2:
            grasp_initial_heuristic_solution(0, 0.5, instance, solution, var_pos);
            break;
        case 3: {
            int first_node = 0;
            int second_node = 1;
            int cost = distance(0, 1, instance);
            int cur_cost;
            for (int k = 0; k < instance->nnode; k++) {
                for (int j = k + 1; j < instance->nnode; j++) {
                    cur_cost = distance(k, j, instance);
                    if (cur_cost > cost) {
                        first_node = k;
                        second_node = j;
                        cost = cur_cost;
                    }
                }
            }
            extra_mileage_initial_heuristic_solution(first_node, second_node, 0.5, instance, solution, var_pos);
            break;
        }
    }
}

void simple_initial_heuristic_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int nnode = instance->nnode;
    for (int i = 0; i < nnode - 1; i++) {
        solution[var_pos(i, i + 1, instance)] = 1.0;
    }
    solution[var_pos(0, nnode - 1, instance)] = 1.0;
}

void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, 2);
    quit_on_GRB_error(instance->env, instance->model, error);

    //find the warm solution to feed to algorithm
    int nvariables = (int) (0.5 * (instance->nnode * instance->nnode - instance->nnode));
    double *solution = calloc(nvariables, sizeof(double));
    get_initial_heuristic_sol(instance, solution, var_pos);

    error = GRBsetdblattrarray(instance->model, GRB_DBL_ATTR_START, 0, nvariables, solution);
    quit_on_GRB_error(instance->env, instance->model, error);

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);
}

/*void naive_initial_heuristic_solution(int selected_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], pred[n_node];

    int count, next_node, cur_node;
    double min_dist = INFINITY;

    cur_node = selected_node;

    next_node = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j =  i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            if (cost[pos] < min_dist && i == cur_node) {
                min_dist = cost[pos];
                next_node = j;
            }
        }
        visited[i] = 0;
    }

    pred[next_node] = 0;
    visited[next_node] = 1;
    cur_node = next_node;
    next_node = 0;
    count = 1;

    while (count < n_node) {

        min_dist = INFINITY;

        for (int i = 0; i < n_node; i++) {
            if (i != cur_node) {
                int pos = var_pos(cur_node, i, instance);
                if (cost[pos] < min_dist && !visited[i]) {
                    min_dist = cost[pos];
                    next_node = i;
                }
            }
        }

        pred[next_node] = cur_node;
        visited[cur_node] = 1;
        cur_node = next_node;
        count++;
    }

    for (int i = 0; i < n_node; i++) {
        solution[var_pos(pred[i], i, instance)] = 1.0;
    }

}*/

void naive_initial_heuristic_solution(int start_node, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], next[n_node];

    int count, next_node, cur_node;
    double min_dist = INFINITY;

    cur_node = start_node;

    next_node = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            if (cost[pos] < min_dist && i == cur_node) {
                min_dist = cost[pos];
                next_node = j;
            }
        }
        visited[i] = 0;
    }

    next[cur_node] = next_node;
    visited[cur_node] = 1;
    cur_node = next_node;
    next_node = 0;
    count = 1;

    while (count < n_node) {

        min_dist = INFINITY;

        for (int i = 0; i < n_node; i++) {
            if (i != cur_node) {
                int pos = var_pos(cur_node, i, instance);
                if (cost[pos] < min_dist && !visited[i]) {
                    min_dist = cost[pos];
                    next_node = i;
                }
            }
        }

        next[cur_node] = next_node;
        visited[cur_node] = 1;
        cur_node = next_node;
        count++;
    }

    for (int i = 0; i < n_node; i++) {
        if (i != next[i]) {
            solution[var_pos(i, next[i], instance)] = 1.0;
        } else {
            solution[var_pos(i, start_node, instance)] = 1.0;
        }
    }

}

//Find the n_node nodes with minimum distance from curr_node
void get_min_distances(int curr_node, double *dist, int *not_available_distance, int *select_node, int n_select_node,
                       int *visited, int *next, Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {

    double min;
    int num_node = instance->nnode;
    int dist_pos;
    int node;

    for (int k = 0; k < n_select_node; k++) {

        min = INFINITY;

        dist_pos = 0;

        node = 0;

        for (int i = 0; i < num_node; i++) {
            if (i != curr_node && i != next[curr_node]) {
                int pos = var_pos(curr_node, i, instance);
                if (dist[pos] < min && !not_available_distance[pos] && !visited[i]) {
                    min = dist[pos];
                    dist_pos = pos;
                    node = i;
                }
            }
        }

        select_node[k] = node;
        not_available_distance[dist_pos] = 1;
    }

    //reset not available distance
    for (int j = 0; j < n_select_node; j++) {
        int pos = var_pos(curr_node, select_node[j], instance);
        not_available_distance[pos] = 0;
    }
}

void grasp_initial_heuristic_solution(int start_node, double p_greedy, Tsp_prob *instance, double *solution,
                                      int (*var_pos)(int, int, Tsp_prob *)) {

    int n_node = instance->nnode;
    int n_edges = (n_node * (n_node - 1)) / 2;
    double cost[n_edges];
    int visited[n_node], next[n_node];
    int num_selected_node = 3;
    int other_node[num_selected_node];
    int not_available_distance[n_edges];

    int count, next_node, cur_node;
    double min_dist;

    cur_node = start_node;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            cost[pos] = distance(i, j, instance);
            not_available_distance[pos] = 0;
        }
        visited[i] = 0;
    }

    next_node = 0;
    count = 0;

    while (count < n_node) {

        min_dist = INFINITY;

        double use_greedy = ((double) rand() / (RAND_MAX));

        if (use_greedy <= p_greedy || count > n_node - num_selected_node) { //greedy approach
            for (int i = 0; i < n_node; i++) {
                if (i != cur_node) {
                    int pos = var_pos(cur_node, i, instance);
                    if (cost[pos] < min_dist && !visited[i]) {
                        min_dist = cost[pos];
                        next_node = i;
                    }
                }
            }
        } else { //random approach
            get_min_distances(cur_node, cost, not_available_distance, other_node, num_selected_node, visited, next,
                              instance, var_pos);
            double prob = ((double) (rand() - 1) / RAND_MAX);
            double gap = 1.0 / num_selected_node;
            int array_pos = ceil(prob / gap);
            next_node = other_node[array_pos - 1];
        }

        next[cur_node] = next_node;
        visited[cur_node] = 1;
        cur_node = next_node;

        count++;
    }

    for (int i = 0; i < n_node; i++) {
        if (i != next[i]) {
            solution[var_pos(i, next[i], instance)] = 1.0;
        } else {
            solution[var_pos(i, start_node, instance)] = 1.0;
        }
    }
}


void extra_mileage_cycle_to_solution(int n_node, int *cur_cycle, double *solution, Tsp_prob *instance,
                                     int (*var_pos)(int, int, Tsp_prob *)) {
    int pos;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            pos = var_pos(i, j, instance);
            solution[pos] = 0.0;
        }
    }

    for (int i = 0; i < n_node - 1; i++) {
        pos = var_pos(cur_cycle[i], cur_cycle[i + 1], instance);
        solution[pos] = 1.0;
        //printf("pos: %d edge: (%d, %d)\n", pos, cur_cycle[i], cur_cycle[i + 1]);
    }
    pos = var_pos(cur_cycle[n_node - 1], cur_cycle[0], instance);
    solution[pos] = 1.0;
    plot_solution_fract(instance, solution, var_pos);
}

void extra_mileage_initial_heuristic_solution(int first_node, int second_node, double p_grasp, Tsp_prob *instance,
                                              double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    int n_node = instance->nnode;
    if (n_node < 2) {
        printf("ERROR: not enough nodes in the instance, from_index extra_milage_initial_heuristic_solution.\n");
        exit(1);
    }
    int n_edges = (n_node * (n_node - 1)) / 2;
    //array of costs, speeds up calculations
    double *costs = calloc(n_edges, sizeof(double));
    //Current cycle being processed
    int *cur_cycle = calloc(n_node, sizeof(double));
    //Next cycle being built
    int *next_cycle = calloc(n_node, sizeof(double));
    //current number of nodes in the cycle
    int cur_nodes;
    //number of nodes in the new cycle being built
    int next_nodes;

    //bitmap of in_cycle nodes
    char *in_cycle = calloc(n_node, sizeof(double));

    //number of nodes to_index consider for selection: the first one is selected with probability p_grasp, the other with (1-p_grasp)/3
    int grasped_nodes = 4;
    //list of grasped nodes
    int *min_list = calloc(grasped_nodes, sizeof(double));
    int nodes_left;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            int pos = var_pos(i, j, instance);
            costs[pos] = distance(i, j, instance);
        }
        in_cycle[i] = 0;
    }

    //Initialize cycle
    cur_nodes = 2;
    cur_cycle[0] = first_node;
    cur_cycle[1] = second_node;
    in_cycle[first_node] = 1;
    in_cycle[second_node] = 1;
    nodes_left = n_node - 2;

    //variables used during cycle construction
    double use_greedy, prob, gap;
    int pos;

    //find the smallest extra-mileage amongst arcs
    char *cur_cycle_replaced = calloc(n_node, sizeof(double));
    cur_cycle_replaced[0] = cur_cycle_replaced[1] = 0;
    int left_to_replace = 2;
    double min_cost = INFINITY;
    int selected_arc_from, selected_arc_to;

    /*while (nodes_left > 0) {
        //as long as I still have nodes to_index visit left
        next_nodes = 0;
        while(left_to_replace > 0) {
            min_cost = INFINITY;
            if (nodes_left == 0) {
                for (int i = 0; i < n_node; i++) {
                    if (!cur_cycle_replaced[i]) {
                        next_cycle[next_nodes++] = cur_cycle[i];
                    }
                }
            } else {
                for (int i = 0; (i < cur_nodes - 1); i++) {
                    if (!cur_cycle_replaced[i]) {
                        extra_mileage_get_mins(cur_cycle[i], cur_cycle[i + 1], costs, min_list, grasped_nodes,
                                               in_cycle, instance, var_pos);
                        use_greedy = ((double) rand() / (RAND_MAX));
                        if (nodes_left < (grasped_nodes)) {
                            use_greedy = 0;
                        }
                        next_cycle[next_nodes++] = cur_cycle[i];
                        if (use_greedy <= p_grasp) {
                            next_cycle[next_nodes++] = min_list[0];
                            in_cycle[min_list[0]] = 1;
                        } else {//select at random from_index other candidates
                            prob = ((double) (rand() - 1) / (RAND_MAX));
                            pos = ceil(prob * (grasped_nodes - 1));
                            next_cycle[next_nodes++] = min_list[pos];
                            in_cycle[min_list[pos]] = 1;
                        }
                        //a node has been added to_index the cycle
                        nodes_left--;
                    }
                }
                //extra_mileage_cycle_to_solution(next_nodes, next_cycle, solution, instance, var_pos);
            }
            if (nodes_left == 0) {
                //I just need to_index add the last node
                next_cycle[next_nodes++] = cur_cycle[cur_nodes - 1];
            } else {
                extra_mileage_get_mins(cur_cycle[cur_nodes - 1], cur_cycle[0], costs, min_list, grasped_nodes,
                                       in_cycle, instance, var_pos);
                use_greedy = ((double) rand() / RAND_MAX);
                if (nodes_left < (grasped_nodes)) {
                    use_greedy = 0;
                }
                next_cycle[next_nodes++] = cur_cycle[cur_nodes - 1];
                if (use_greedy <= p_grasp) {
                    next_cycle[next_nodes++] = min_list[0];
                    in_cycle[min_list[0]] = 1;
                } else {//select at random from_index other candidates
                    prob = ((double) (rand() - 1) / RAND_MAX);
                    pos = ceil(prob * (grasped_nodes - 1));
                    next_cycle[next_nodes++] = min_list[pos];
                    in_cycle[min_list[pos]] = 1;
                }
                nodes_left--;
            }
        }

        for (int i = 0; i < next_nodes; i++) {
            cur_cycle[i] = next_cycle[i];
        }
        cur_nodes = next_nodes;
        printf("cur nodes: %d \n", cur_nodes);
    }*/

    int from_index, to_index;
    double selected_cost;
    int selected_extra_node, cur_extra_node;
    int arc0, arc1, arc2;
    while (nodes_left > 0) {
        min_cost = INFINITY;
        //select the best arc
        for (int i = 0; i < cur_nodes; i++) {
            from_index = i;
            if (i < (cur_nodes - 1)) {
                to_index = i + 1;
            } else {
                to_index = 0;
            }
            extra_mileage_get_mins(cur_cycle[from_index], cur_cycle[to_index], costs, min_list, grasped_nodes,
                                   in_cycle, instance, var_pos);

            use_greedy = ((double) rand() / RAND_MAX);
            if (nodes_left < (grasped_nodes)) {
                use_greedy = 0;
            }
            if (use_greedy <= p_grasp) {
                cur_extra_node = 0;
            } else {//select at random from_index other candidates
                prob = ((double) (rand() - 1) / RAND_MAX);
                pos = ceil(prob * (grasped_nodes - 1));
                cur_extra_node = pos;
            }
            arc0 = var_pos(cur_cycle[from_index], cur_cycle[to_index], instance);
            arc1 = var_pos(cur_cycle[from_index], min_list[cur_extra_node], instance);
            arc2 = var_pos(min_list[cur_extra_node], cur_cycle[to_index], instance);
            selected_cost = costs[arc1] + costs[arc2] - costs[arc0];

            if (selected_cost < min_cost) {
                min_cost = selected_cost;
                selected_arc_from = from_index;
                selected_arc_to = to_index;
                selected_extra_node = min_list[cur_extra_node];
            }
        }
        in_cycle[selected_extra_node] = 1;
        next_nodes = 0;
        for (int i = 0; i < cur_nodes; i++) {
            next_cycle[next_nodes++] = cur_cycle[i];
            if (i == selected_arc_from) {
                next_cycle[next_nodes++] = selected_extra_node;
            }
        }

        for (int i = 0; i < next_nodes; i++) {
            cur_cycle[i] = next_cycle[i];
        }
        cur_nodes = next_nodes;
        nodes_left--;
        //extra_mileage_cycle_to_solution(cur_nodes, cur_cycle, solution, instance, var_pos);
    }

    extra_mileage_cycle_to_solution(cur_nodes, cur_cycle, solution, instance, var_pos);
}

void
extra_mileage_insert_sorted_nodes(double cost, int node_pos, int *min_list, double *min_costs_list, int list_size) {
    int k = list_size - 1;
    int temp_element;
    double temp_cost;

    //set element to last if the cost is not too big
    if (min_costs_list[list_size - 1] <= cost) {
        return;
    } else {
        min_costs_list[list_size - 1] = cost;
        min_list[list_size - 1] = node_pos;
    }

    //start swapping until I find the right place for this guy
    while (k > 0 && min_costs_list[k - 1] > min_costs_list[k]) {
        //swap nodes
        temp_element = min_list[k];
        min_list[k] = min_list[k - 1];
        min_list[k - 1] = temp_element;

        //swap costs
        temp_cost = min_costs_list[k];
        min_costs_list[k] = min_costs_list[k - 1];
        min_costs_list[k - 1] = temp_cost;

        k--;
    }
}

void
extra_mileage_get_mins(int first_node, int second_node, double *costs, int *min_list, int grasped_nodes, char *visited,
                       Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {

    int pos_xj;
    int pos_jy;
    double min_costs_list[grasped_nodes];

    for (int i = 0; i < grasped_nodes; i++) {
        min_costs_list[i] = INFINITY;
    }

    for (int i = 0; i < instance->nnode; i++) {
        if (!visited[i]) {
            pos_xj = var_pos(first_node, i, instance);
            pos_jy = var_pos(i, second_node, instance);
            extra_mileage_insert_sorted_nodes(costs[pos_xj] + costs[pos_jy], i, min_list, min_costs_list,
                                              grasped_nodes);
        }
    }


}