#include "meta_heuristic_utils.h"


int x_pos_metaheuristic(int i, int j, Tsp_prob *instance);

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence);

void
three_opt_swap_rnd(Tsp_prob *instance, int *node_sequence, int i, int j, int k, int n_node, int *new_node_sequence);

int two_opt(Tsp_prob *instance, double *solution, int *node_sequence, int *costs) {
    int n_node = instance->nnode;
    int coord1, coord2, coord3, coord4;
    int *new_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *new_node_sequence = new_sequence_allocation;
    int *temp_pointer_to_int;
    int *cur_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *cur_sequence = cur_sequence_allocation;

    get_node_path(solution, cur_sequence, instance);
    //get_node_path(solution, new_node_sequence, instance);
    for (int i = 0; i < n_node + 1; i++) {
        new_node_sequence[i] = cur_sequence[i];
    }
    int best_distance = compute_total_distance(instance, cur_sequence);
    int new_distance = 0;
    char improve = 0;
    do {
        //best_distance = compute_total_distance(instance, node_sequence);
        improve = 0;
        for (int i = 0; i < n_node - 2; i++) {
            for (int k = i + 2; k < n_node; k++) {
                coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
                coord2 = x_pos_metaheuristic(cur_sequence[k], cur_sequence[k + 1], instance);
                coord3 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[k], instance);
                coord4 = x_pos_metaheuristic(cur_sequence[i + 1], cur_sequence[k + 1], instance);
                new_distance = best_distance -
                               costs[coord1] -
                               costs[coord2] +
                               costs[coord3] +
                               costs[coord4];

                if (new_distance < best_distance) {
                    improve = 1;
                    two_opt_swap(cur_sequence, i, k, n_node, new_node_sequence);
                    //copy_node_sequence(cur_sequence, new_node_sequence, n_node + 1);
                    temp_pointer_to_int = cur_sequence;
                    cur_sequence = new_node_sequence;
                    new_node_sequence = temp_pointer_to_int;
                    best_distance = new_distance;
                }
            }

        }
        //printf("improved \n");
    } while (improve == 1);

    copy_node_sequence(node_sequence, cur_sequence, n_node + 1);
    //new_solution(instance, node_sequence, solution);

    free(new_sequence_allocation);
    free(cur_sequence_allocation);
    return best_distance;
}

int two_opt_f(Tsp_prob *instance, int *node_sequence, int *costs) {
    int n_node = instance->nnode;
    int coord1, coord2, coord3, coord4;
    int *new_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *new_node_sequence = new_sequence_allocation;
    int *temp_pointer_to_int;
    int *cur_sequence = node_sequence;

    int best_distance = compute_total_distance(instance, cur_sequence);
    int new_distance = 0;
    char improve = 0;
    do {
        //best_distance = compute_total_distance(instance, node_sequence);
        improve = 0;
        for (int i = 0; i < n_node - 2; i++) {
            coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
            for (int k = i + 2; k < n_node; k++) {
                coord2 = x_pos_metaheuristic(cur_sequence[k], cur_sequence[k + 1], instance);
                coord3 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[k], instance);
                coord4 = x_pos_metaheuristic(cur_sequence[i + 1], cur_sequence[k + 1], instance);
                new_distance = best_distance +
                               costs[coord3] +
                               costs[coord4] -
                               costs[coord1] -
                               costs[coord2];

                if (new_distance < best_distance) {
                    improve = 1;
                    two_opt_swap(cur_sequence, i, k, n_node, new_node_sequence);
                    //copy_node_sequence(cur_sequence, new_node_sequence, n_node + 1);
                    temp_pointer_to_int = cur_sequence;
                    cur_sequence = new_node_sequence;
                    new_node_sequence = temp_pointer_to_int;
                    best_distance = new_distance;
                    coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
                }
            }

        }
        //printf("improved \n");
    } while (improve == 1);

    free(new_sequence_allocation);
    return best_distance;
}

int two_opt_dlb(Tsp_prob *instance, double *solution, int *node_sequence, int *costs) {
    int n_node = instance->nnode;
    int coord1, coord2, coord3, coord4;
    int *new_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *new_node_sequence = new_sequence_allocation;
    int *temp_pointer_to_int;
    int *cur_sequence_allocation = calloc(n_node + 1, sizeof(int));
    int *cur_sequence = cur_sequence_allocation;

    get_node_path(solution, cur_sequence, instance);
    int best_distance = compute_total_distance(instance, cur_sequence);
    int new_distance = 0;
    char improve = 0;

    char *dlb = calloc(n_node, sizeof(char));
    char dlb_improv = 0;
    int i;
    do {
        //best_distance = compute_total_distance(instance, node_sequence);
        improve = 0;

        for (int index = 1; index < n_node; index++) {
            for (int dir = 0; dir < 2; dir++) {
                i = index - dir;
                coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
                if (dlb[cur_sequence[i]] == 1) {
                    continue;
                }
                for (int k = 0; k < n_node; k++) {
                    if (i == k || i == k + 1 || i + 1 == k) {
                        continue;
                    }
                    coord2 = x_pos_metaheuristic(cur_sequence[k], cur_sequence[k + 1], instance);
                    coord3 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[k], instance);
                    coord4 = x_pos_metaheuristic(cur_sequence[i + 1], cur_sequence[k + 1], instance);
                    new_distance = best_distance +
                                   costs[coord3] +
                                   costs[coord4] -
                                   costs[coord1] -
                                   costs[coord2];

                    if (new_distance < best_distance) {
                        dlb[cur_sequence[i]] = 0;
                        dlb[cur_sequence[i + 1]] = 0;
                        dlb[cur_sequence[k]] = 0;
                        dlb[cur_sequence[k + 1]] = 0;
                        dlb_improv = 1;
                        improve = 1;
                        two_opt_swap(cur_sequence, i, k, n_node, new_node_sequence);
                        //copy_node_sequence(cur_sequence, new_node_sequence, n_node + 1);
                        temp_pointer_to_int = cur_sequence;
                        cur_sequence = new_node_sequence;
                        new_node_sequence = temp_pointer_to_int;
                        best_distance = new_distance;
                        coord1 = x_pos_metaheuristic(cur_sequence[i], cur_sequence[i + 1], instance);
                    }
                }

            }
            dlb[cur_sequence[index]] = 1;
        }
        //printf("improved \n");
    } while (improve == 1);

    copy_node_sequence(node_sequence, cur_sequence, n_node + 1);
    //new_solution(instance, node_sequence, solution);

    free(new_sequence_allocation);
    free(cur_sequence_allocation);
    return best_distance;
}

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance) {
    int n_node = instance->nnode;
    char available[n_node];
    int count = 0;

    for (int i = 0; i < n_node; i++) {
        available[i] = 1;
    }

    int curr_node = 0;
    node_sequence[0] = node_sequence[n_node] = 0;
    int l = 1;
    available[0] = 0;

    while (count < n_node) {
        for (int i = 0; i < n_node; i++) {
            if (i != curr_node) {
                if (available[i] && solution[x_pos_metaheuristic(curr_node, i, instance)] > 1 - TOLERANCE) {
                    node_sequence[l] = i;
                    available[curr_node] = 0;
                    curr_node = i;
                    l++;
                    break;
                }
            }
        }

        count++;
    }
}

int x_pos_metaheuristic(int i, int j, Tsp_prob *instance) {
    if (i > j) {
        return j * instance->nnode + i - ((j + 1) * (j + 2)) / 2;
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

int compute_total_distance(Tsp_prob *instance, int *node_sequence) {
    int dist = 0;

    for (int i = 0; i < instance->nnode; i++) {
        dist += distance(node_sequence[i], node_sequence[i + 1], instance);
    }

    return dist;
}

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence) {
    int l = 0;
    if (i > k) {
        int swap = i;
        i = k;
        k = swap;
    }
    for (int c = 0; c <= i; c++) {
        new_node_sequence[l++] = node_sequence[c];
    }
    for (int c = k; c >= i + 1; c--) {
        new_node_sequence[l++] = node_sequence[c];
    }
    for (int c = k + 1; c < n_node + 1; c++) {
        new_node_sequence[l++] = node_sequence[c];
    }
}

void copy_node_sequence(int *to_node_sequence, int *from_node_sequence, int n_node) {

    for (int i = 0; i <= n_node; i++) {
        //printf("NODE: %d, %d\n", i, from_node_sequence[i]);
        to_node_sequence[i] = from_node_sequence[i];
    }
}

void kick(Tsp_prob *instance, int *node_sequence, int n_node, int *incumbent_node_sequence) {

    //printf("Kick!!!\n");
    //plot_solution_fract(instance, solution, x_pos_metaheuristic);

    int selected_element = 0;
    int *not_available = calloc(n_node, sizeof(int));

    int node_pos[3] = {0};

    int selected_node;
    char select_this;

    while (selected_element < 3) {
        select_this = 0;
        selected_node = ((int) rand() % (n_node));
        if (selected_node == (n_node - 1)) {
            if (!not_available[0] && !not_available[n_node - 1] && !not_available[n_node - 2]) {
                select_this = 1;
            }
        } else if (selected_node == 0) {
            if (!not_available[1] && !not_available[0] && !not_available[n_node - 1]) {
                select_this = 1;
            }
        } else {
            if (!not_available[selected_node + 1] && !not_available[selected_node] &&
                !not_available[selected_node - 1]) {
                select_this = 1;
            }
        }
        if (select_this) {
            node_pos[selected_element] = selected_node;
            selected_element++;
            not_available[selected_node] = 1;
        }

    }
    char flag = 1;
    int swap;
    while (flag) {
        flag = 0;
        for (int i = 0; i < 2; i++) {
            if (node_pos[i] > node_pos[i + 1]) {
                swap = node_pos[i + 1];
                node_pos[i + 1] = node_pos[i];
                node_pos[i] = swap;
                flag = 1;
            }
        }
    }

    three_opt_swap_rnd(instance, incumbent_node_sequence, node_pos[0], node_pos[1], node_pos[2], n_node, node_sequence);

    //plot_solution_fract(instance, solution, x_pos_metaheuristic);

    free(not_available);

}

void new_solution(Tsp_prob *instance, int *input_sequence, double *output_solution) {

    int n_node = instance->nnode;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            int pos = x_pos_metaheuristic(i, j, instance);
            output_solution[pos] = 0;
        }
    }


    for (int i = 0; i < n_node; i++) {
        //printf("cur: %d %d\n",input_sequence[i], input_sequence[i+1] );
        int pos = x_pos_metaheuristic(input_sequence[i], input_sequence[i + 1], instance);
        output_solution[pos] = 1;
    }
}

int link_from_to(int *node_sequence, int *new_node_sequence, int from, int to, int cur_new_sequence_len, int reverse) {
    int seqlen = cur_new_sequence_len;
    int temp;
    if (reverse) {
        //I want from > to
        if (from < to) {
            temp = from;
            from = to;
            to = temp;
        }
        for (int i = from; i >= to; i--) {
            //printf("linking: %d,  %d, %d\n", seqlen, i, node_sequence[i]);
            new_node_sequence[seqlen++] = node_sequence[i];
        }
    } else {
        //I want from < to
        if (from > to) {
            temp = from;
            from = to;
            to = temp;
        }
        for (int i = from; i <= to; i++) {
            //printf("linking: %d,  %d, %d\n", seqlen, i, node_sequence[i]);
            new_node_sequence[seqlen++] = node_sequence[i];
        }
    }

    return seqlen;

}

void
three_opt_swap_rnd(Tsp_prob *instance, int *node_sequence, int i, int j, int k, int n_node, int *new_node_sequence) {
    int number = (int) (genrand64_real2() * 4);
    int new_sequence_length = 0;
    new_sequence_length = link_from_to(node_sequence, new_node_sequence, 0, i, new_sequence_length, 0);
    switch (number) {
        case 0: {
            //new_node_sequence[new_sequence_length++] = node_sequence[j];
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, j, i + 1, new_sequence_length, 1);
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, k, j + 1, new_sequence_length, 1);
        }
            break;
        case 1: {
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, k, j + 1, new_sequence_length, 1);
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, i + 1, j, new_sequence_length, 0);
        }
            break;
        case 2: {
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, j + 1, k, new_sequence_length, 0);
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, j, i + 1, new_sequence_length, 1);
        }
            break;
        case 3: {
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, j + 1, k, new_sequence_length, 0);
            new_sequence_length = link_from_to(node_sequence, new_node_sequence, i + 1, j, new_sequence_length, 0);
        }
            break;
    }
    new_sequence_length = link_from_to(node_sequence, new_node_sequence, k + 1, n_node, new_sequence_length, 0);

    /*int dist_1 = distance(a, b, instance) + distance(c, d, instance) + distance(e, f, instance);
    int dist_2 = distance(a, c, instance) + distance(b, d, instance) + distance(e, f, instance);
    int dist_3 = distance(a, b, instance) + distance(c, e, instance) + distance(d, f, instance);
    int dist_4 = distance(a, d, instance) + distance(e, b, instance) + distance(c, f, instance);
    int dist_5 = distance(f, b, instance) + distance(c, d, instance) + distance(e, a, instance);

    if (dist_1 > dist_2) {
        two_opt_swap(node_sequence, i, j, n_node, new_node_sequence);
    } else if (dist_1 > dist_3) {
        two_opt_swap(node_sequence, j, k, n_node, new_node_sequence);
    } else if (dist_1 > dist_5) {
        two_opt_swap(node_sequence, i, k, n_node, new_node_sequence);
    } else if (dist_1 > dist_4) {
        int *tmp = calloc(n_node, sizeof(int));
        two_opt_swap(node_sequence, i, j, n_node, tmp);
        two_opt_swap(tmp, j, k, n_node, new_node_sequence);
        free(tmp);
    }*/
}

void double_bridge_kick(Tsp_prob *instance, int *output_sequence, int n_node, int *input_sequence) {
    copy_node_sequence(output_sequence, input_sequence, n_node);
    int node_pos[4] = {0};

    int selected_node;
    char select_this;
    int selected_element = 0;
    int *not_available = calloc(n_node, sizeof(int));

    while (selected_element < 4) {
        select_this = 0;
        selected_node = ((int) (genrand64_int64() % (n_node - 1)));
        if (selected_node == 0) {
            if (!not_available[1] && !not_available[0] && !not_available[n_node - 1]) {
                select_this = 1;
            }
        } else {
            if (!not_available[selected_node + 1] && !not_available[selected_node] &&
                !not_available[selected_node - 1]) {
                select_this = 1;
            }
        }
        if (select_this) {
            node_pos[selected_element] = selected_node;
            selected_element++;
            not_available[selected_node] = 1;
        }

    }
    char flag = 1;
    int swap;
    while (flag) {
        flag = 0;
        for (int i = 0; i < 3; i++) {
            if (node_pos[i] > node_pos[i + 1]) {
                swap = node_pos[i + 1];
                node_pos[i + 1] = node_pos[i];
                node_pos[i] = swap;
                flag = 1;
            }
        }
    }

    int temp = 0;
    temp = output_sequence[node_pos[0] + 1];
    output_sequence[node_pos[0] + 1] = output_sequence[node_pos[2] + 1];
    output_sequence[node_pos[2] + 1] = temp;

    temp = output_sequence[node_pos[1] + 1];
    output_sequence[node_pos[1] + 1] = output_sequence[node_pos[3] + 1];
    output_sequence[node_pos[3] + 1] = temp;

    free(not_available);
}