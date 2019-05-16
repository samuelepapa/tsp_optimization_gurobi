#include "meta_heuristic_utils.h"


int x_pos(int i, int j, Tsp_prob *instance);

void two_opt_swap(int *node_sequence, int i, int k, int n_node, int *new_node_sequence);

void assign_new_node_sequence(int *node_sequence, int *new_node_sequence, int n_node);

void three_opt_swap(Tsp_prob *instance, int *node_sequence, int i, int j, int k, int n_node, int *new_node_sequence);


void two_opt(Tsp_prob *instance, double *solution, int *node_sequence) {
    int n_node = instance->nnode;

    int *new_node_sequence = calloc(n_node + 1, sizeof(int));
    int best_distance = 0;
    int delta = 0;

    get_node_path(solution, node_sequence, instance);

    do {
        best_distance = compute_total_distance(instance, node_sequence);
        for (int i = 1; i < n_node - 1; i++) {
            for (int k = i + 1; k < n_node; k++) {
                two_opt_swap(node_sequence, i, k, n_node + 1, new_node_sequence);
                int new_distance = compute_total_distance(instance, new_node_sequence);
                delta = new_distance - best_distance;
                if (delta < 0) {
                    assign_new_node_sequence(node_sequence, new_node_sequence, n_node + 1);
                    best_distance = new_distance;
                    printf("Distance found: %d \n", best_distance);
                }
            }

        }
    } while(delta <= 0);

    new_solution(instance, node_sequence, solution);

    free(new_node_sequence);
}

void get_node_path(double *solution, int *node_sequence, Tsp_prob *instance) {
    int n_node = instance->nnode;
    int available[n_node];
    int count = 0;

    for (int i = 0; i < n_node; i++) {
        available[i] = 1;
    }

    int curr_node = 0;
    node_sequence[0] = node_sequence[n_node] = 0;
    int l = 1;

    while (count < n_node) {

        for (int i = 0; i < n_node; i++) {
            if (i != curr_node) {
                if (solution[x_pos(curr_node, i, instance)] > 1 - TOLERANCE && available[i]) {
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

    int dist = 0;

    for (int i = 0; i < instance->nnode; i++) {
        dist += distance(node_sequence[i], node_sequence[i + 1], instance);
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

void kick(Tsp_prob *instance, double *solution, int n_node) {

    printf("Kick!!!");

    int selected_element = 0;
    int node_sequence[n_node + 1];
    int *not_available = calloc(n_node, sizeof(int));

    int *new_node_sequence = calloc(n_node + 1, sizeof(int));

    get_node_path(solution, node_sequence, instance);

    int node_pos[3] = {0};

    srand((unsigned) time(NULL));

    while(selected_element < 3) {
        int  selected_node = (int) rand() % n_node;
        if (!not_available[selected_node]) {
            node_pos[selected_element] = selected_node;
            selected_element++;
            not_available[selected_node] = 1;
        }

    }

    three_opt_swap(instance, node_sequence, node_pos[0], node_pos[1], node_pos[2], n_node + 1, new_node_sequence);

    new_solution(instance, new_node_sequence, solution);

    free(not_available);

    free(new_node_sequence);
}

void new_solution(Tsp_prob *instance, int *best_solution, double *solution) {

    int n_node = instance->nnode;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            int pos = x_pos(i, j, instance);
            solution[pos] = 0;
        }
    }


    for (int i = 0; i < n_node; i++) {
        int pos = x_pos(best_solution[i], best_solution[i + 1], instance);
        solution[pos] = 1;
    }
}

void three_opt_swap(Tsp_prob *instance, int *node_sequence, int i, int j, int k, int n_node, int *new_node_sequence) {
    int a = node_sequence[i - 1];
    int b = node_sequence[i];
    int c = node_sequence[j - 1];
    int d = node_sequence[j];
    int f = node_sequence[k - 1];
    int e = node_sequence[k % (n_node + 1)];

    int dist_1 = distance(a, b, instance) + distance(c, d, instance) + distance(e, f, instance);
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
    }
}