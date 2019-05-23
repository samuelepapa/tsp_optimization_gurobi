#include "meta_heuristic_utils.h"


int x_pos_metaheuristic(int i, int j, Tsp_prob *instance);

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence);

void copy_node_sequence(int *to_node_sequence, int *from_node_sequence, int n_node);

void
three_opt_swap_rnd(Tsp_prob *instance, int *node_sequence, int i, int j, int k, int n_node, int *new_node_sequence);


void two_opt(Tsp_prob *instance, double *solution, int *node_sequence) {
    int n_node = instance->nnode;

    int *new_node_sequence = calloc(n_node + 1, sizeof(int));
    int delta = 0;

    get_node_path(solution, node_sequence, instance);

    int best_distance = compute_total_distance(instance, node_sequence);
    int new_distance = 0;

    int improve = 0;
    do {
        //best_distance = compute_total_distance(instance, node_sequence);
        improve = 0;
        for (int i = 0; i < n_node - 2; i++) {
            for (int k = i + 2; k < n_node; k++) {
                two_opt_swap(node_sequence, i, k, n_node, new_node_sequence);
                new_distance = compute_total_distance(instance, new_node_sequence);
                delta = new_distance - best_distance;
                if (delta < 0) {
                    improve = 1;
                    copy_node_sequence(node_sequence, new_node_sequence, n_node + 1);
                    best_distance = new_distance;
                }
            }

        }
        printf("improved \n");
    } while (improve == 1);

    //new_solution(instance, node_sequence, solution);

    free(new_node_sequence);
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
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return x_pos_metaheuristic(j, i, instance);
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

    for (int i = 0; i < n_node; i++) {
        to_node_sequence[i] = from_node_sequence[i];
    }
}

void kick(Tsp_prob *instance, double *solution, int n_node, double *input_solution) {

    printf("Kick!!!\n");
    //plot_solution_fract(instance, solution, x_pos_metaheuristic);

    int selected_element = 0;
    int node_sequence[n_node + 1];
    int *not_available = calloc(n_node + 1, sizeof(int));

    int *new_node_sequence = calloc(n_node + 1, sizeof(int));

    get_node_path(input_solution, node_sequence, instance);

    int node_pos[3] = {0};

    int selected_node;
    char select_this;

    while (selected_element < 3) {
        select_this = 0;
        selected_node = ((int) rand() % (n_node));
        if (selected_node == n_node) {
            if (!not_available[0] && !not_available[n_node] && !not_available[n_node - 1]) {
                select_this = 1;
            }
        } else if (selected_node == 0) {
            if (!not_available[selected_node + 1] && !not_available[selected_node] && !not_available[n_node]) {
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

    three_opt_swap_rnd(instance, node_sequence, node_pos[0], node_pos[1], node_pos[2], n_node, new_node_sequence);

    new_solution(instance, new_node_sequence, solution);

    //plot_solution_fract(instance, solution, x_pos_metaheuristic);

    free(not_available);

    free(new_node_sequence);
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
    int number = (int) ((((double) (rand() - 1)) / RAND_MAX) * 4);
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