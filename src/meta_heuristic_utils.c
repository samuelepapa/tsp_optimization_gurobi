#include "meta_heuristic_utils.h"

void get_node_path (double *solution, int *node_sequence, Tsp_prob *instance);

int x_pos(int i, int j, Tsp_prob *instance);

int compute_total_distance(Tsp_prob *instance, int *node_sequence);

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence);

void assign_new_node_sequence(int *node_sequence, int *new_node_sequence, int n_node);

void two_opt(Tsp_prob *instance, double *solution) {
    int n_node = instance->nnode;
    int node_sequence[n_node];
    int best_distance = 0;
    int improve = 0;

    get_node_path(solution, node_sequence, instance);

    while (improve < 800) {
        best_distance = compute_total_distance(instance, node_sequence);
        for (int i = 1; i < n_node - 1; i++) {
            for (int k = i + 1; k < n_node; k++) {
                int new_node_sequence[n_node];
                two_opt_swap(node_sequence, i, k, n_node, new_node_sequence);
                int new_distance = compute_total_distance(instance, new_node_sequence);
                if (new_distance < best_distance) {
                    improve = 0;
                    assign_new_node_sequence(node_sequence, new_node_sequence, n_node);
                    best_distance = new_distance;
                    printf("Distance found: %d \n", best_distance);
                }
            }

        }

        improve++;
    }

}

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance) {
    int n_node = instance->nnode;
    int available[n_node];
    int count = 0;

    for (int i = 0; i < n_node; i++) {
        available[i] = 1;
    }

    int curr_node = 0;

    while (count < n_node - 1) {

        for (int i = 0; i < n_node; i++) {
            if (i != curr_node) {
                if (solution[x_pos(curr_node, i, instance)] > 1 - TOLERANCE && available[i]) {
                    node_sequence[curr_node] = i;
                    available[curr_node] = 0;
                    curr_node = i;
                    break;
                }
            }
        }

        count++;
    }

    available[0] = 1;

    for (int i = 0; i < n_node; i++) {
        if (i != curr_node) {
            if (solution[x_pos(curr_node, i, instance)] > 1 - TOLERANCE && available[i]) {
                node_sequence[curr_node] = i;
                available[curr_node] = 0;
                break;
            }
        }
    }
}

int x_pos(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return x_pos(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

int compute_total_distance(Tsp_prob *instance, int *node_sequence) {

    int cur_node = 0;
    int count = 0;

    int dist = 0;

    while (count < instance->nnode) {
        int next_node = node_sequence[cur_node];
        dist += distance(cur_node, next_node, instance);
        cur_node = next_node;
        count++;
    }

    return dist;
}

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence) {

    for (int c = 0; c < i; c++) {
        new_node_sequence[c] = node_sequence[c];
    }

    int decr = 0;

    for (int c = i; c < k + 1; c++) {
        new_node_sequence[c] = node_sequence[k - decr];
        decr++;
    }

    for (int c = k + 1; c < n_node; c++) {
        new_node_sequence[c] = node_sequence[c];
    }
}

void assign_new_node_sequence(int *node_sequence, int *new_node_sequence, int n_node) {

    for (int i = 0; i < n_node; i++) {
        node_sequence[i] = new_node_sequence[i];
    }
}