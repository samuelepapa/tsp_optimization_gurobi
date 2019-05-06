#include "union_find.h"


/**
 * Function that does the union of two sets of x and y using union by rank
 * @param conn_comps Array of connected components (subsets)
 * @param x_set First subset
 * @param y_set Second subset
 */
void union_by_rank(Connected_component *conn_comps, int x_set, int y_set);


/**
 * Function that does the union of two sets of x and y using union by size
 * @param conn_comps Array of connected components (subsets)
 * @param x_set First subset
 * @param y_set Second subset
 */
void union_by_size(Connected_component *conn_comps, int x, int y);

/**
 * Control if all the nodes have the correct root
 * @param conn_comps The array of nodes with connected component data
 * @param n_node The number of nodes of the problem
 */
void control_root(Connected_component *conn_comps, int n_node);

void create_graph_u_f(Tsp_prob *instance, Graph *graph) {
    int n_node = instance->nnode;
    graph->V = n_node; //number of nodes
    graph->E = (n_node * (n_node - 1)) / 2; //number of edges
    graph->edge = (Edge *) calloc(graph->E, sizeof(Edge));

    int l = 0;

    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            graph->edge[l].src = i;
            graph->edge[l].dest = j;
            l++;
        }
    }
}

/*int union_find(Graph *graph, double *solution, int (*var_pos)(int, int, Tsp_prob *), Tsp_prob *instance, Connected_component conn_comps[]) {

    int n_node = graph->V;
    int n_edge = graph->E;

    int number_of_comps = n_node;

    for (int v = 0; v < n_node; v++) {
        conn_comps[v].parent = v;
        conn_comps[v].rank = 0;
        conn_comps[v].size = 1;
    }

    for (int e = 0; e < n_edge; e++) {

        int src = graph->edge[e].src;
        int dest = graph->edge[e].dest;

        if (solution[var_pos(src, dest, instance)] > 1-TOLERANCE) {
            int x = find(conn_comps, src);
            int y = find(conn_comps, dest);
            if (x != y) {
                //union_by_rank(conn_comps, x, y);
                union_by_size(conn_comps, x, y);
                number_of_comps--;
            }
        }
    }

    control_root(conn_comps, n_node);


    for (int i = 0; i < n_node; i++) {
        printf("Node %d in component of root %d with rank %d and size %d\n", i, conn_comps[i].parent, conn_comps[i].rank, conn_comps[i].size);
    }

    return number_of_comps;
}*/

/*int find(Connected_component *conn_comps, int i) {

    // find root and make root as parent of i (path compression)
    if (conn_comps[i].parent != i) {
        conn_comps[i].parent = find(conn_comps, conn_comps[i].parent);
    }

    return conn_comps[i].parent;
}

void union_by_rank(Connected_component *conn_comps, int x_set, int y_set) {

    //find the root of the set
    int x_root = find(conn_comps, x_set);
    int y_root = find(conn_comps, y_set);

    int x_rank = conn_comps[x_root].rank;
    int y_rank = conn_comps[y_root].rank;

    if (x_root == y_root) {
        return;
    }

    //attach smaller rank tree under the root of the bigger rank tree (this is the union by rank)
    if (y_rank < x_rank) {
        conn_comps[y_root].parent = x_root;
        //conn_comps[x_root].size += conn_comps[y_root].size;
    } else if (x_rank < y_rank) {
        conn_comps[x_root].parent = y_root;
        //conn_comps[y_root].size += conn_comps[x_root].size;
    } else {
        conn_comps[y_root].parent = x_root;
        conn_comps[x_root].rank++;
        conn_comps[x_root].size += conn_comps[y_root].size;
    }
}*/

void union_by_size(Connected_component *conn_comps, int x, int y) {
    //int x_root = find(conn_comps, x);
    //int y_root = find(conn_comps, y);
    int x_root = x;
    int y_root = y;


    if (x_root == y_root) {
        return;
    }

    if (conn_comps->size[x_root] < conn_comps->size[y_root]) {
        int tmp = y_root;
        y_root = x_root;
        x_root = tmp;
    }

    conn_comps->parent[y_root] = x_root;
    conn_comps->size[x_root] = conn_comps->size[x_root] + conn_comps->size[y_root];

}

void control_root(Connected_component *conn_comps, int n_node) {

    for (int i = 0; i < n_node; i++) {
        conn_comps->parent[i] = find(conn_comps, conn_comps->parent[i]);
    }
}


/**
 * Control if the root was found before
 * @param root_cc Array of connected component roots
 * @param curr_root The current connected component root
 * @param num_comp Number of connected components
 * @return 1 if the current root is in the root array, 0 otherwise
 */
int has_root(int root_cc[], int curr_root, int num_comp) {

    for (int i = 0; i < num_comp; i++) {
        if (curr_root == root_cc[i]) {
            return 1;
        }
    }

    return 0;
}

/*void get_root(int root_cc[], int number_of_comps, Connected_component conn_comps[], int n_node) {

    //initialize the array
    for (int i = 0; i < number_of_comps; i++) {
        root_cc[i] = -1;
    }

    int t = 0;

    for (int i = 0; i < n_node; i++) {
        int cc = conn_comps[i].parent;
        if (has_root(root_cc, cc, number_of_comps) != 0) {
            continue;
        }
        root_cc[t] = cc;
        t++;
    }
}*/


int find(Connected_component *conn_comps, int i) {
    //path splitting better efficiency
    /*while (conn_comps->parent[i] != i) {
        int next = conn_comps->parent[i];
        conn_comps->parent[i] = conn_comps->parent[next];
        i = next;
    }
    return i;*/
    //path halving better efficiency
    while (conn_comps->parent[i] != i) {
        int next = conn_comps->parent[i];
        conn_comps->parent[i] = conn_comps->parent[next];
        i = conn_comps->parent[i];
    }
    return i;
    //path compression
    /*if (conn_comps->parent[i] != i) {
        conn_comps->parent[i] = find(conn_comps, conn_comps->parent[i]);
    }

    return conn_comps->parent[i];*/
}

int find_root(Connected_component *conn_comps, int i) {
    int root = conn_comps->parent[i];

    while (root != i) {
        int next = conn_comps->parent[i];
        root = conn_comps->parent[next];
        i = next;
    }

    return i;
}

/*void union_by_rank (Connected_component *conn_comps, int x, int y) {
    int x_root = find(conn_comps, conn_comps->parent[x]);
    int y_root = find(conn_comps, conn_comps->parent[y]);

    if (x_root == y_root) {
        return;
    }

    if (conn_comps->rank[x_root] < conn_comps->rank[y_root]) {
        conn_comps->parent[x_root] = y_root;
        conn_comps->size[y_root] = conn_comps->size[x_root] + conn_comps->size[y_root];
    } else if (conn_comps->rank[y_root] < conn_comps->rank[x_root]) {
        conn_comps->parent[y_root] = x_root;
        conn_comps->size[x_root] = conn_comps->size[x_root] + conn_comps->size[y_root];
    } else {
        conn_comps->parent[y_root] = x_root;
        conn_comps->rank[x_root]++;
        conn_comps->size[x_root] = conn_comps->size[x_root] + conn_comps->size[y_root];
    }
}*/

int union_find(Graph *graph, double *solution, int (*var_pos)(int, int, Tsp_prob *), Tsp_prob *instance,
               Connected_component *conn_comps) {
    int nnode = instance->nnode;
    int n_conn_comps = nnode;

    for (int i = 0; i < nnode; i++) {
        conn_comps->parent[i] = i;
        conn_comps->rank[i] = 0;
        conn_comps->size[i] = 1;
    }

    for (int e = 0; e < graph->E; e++) {
        int src = graph->edge[e].src;
        int dest = graph->edge[e].dest;
//        if (solution[var_pos(src, dest, instance)] > 1 - TOLERANCE) {
//      This change is due to the fact that we want to find connected components also in non-integer solutions
        if (solution[var_pos(src, dest, instance)] > TOLERANCE) {
            int i_root = find(conn_comps, conn_comps->parent[src]);
            int j_root = find(conn_comps, conn_comps->parent[dest]);
            if (i_root != j_root) {
                //union_by_rank(conn_comps, i_root, j_root);
                union_by_size(conn_comps, i_root, j_root);
                n_conn_comps--;
            }

        }
    }


    /*for (int i = 0; i < nnode; i++) {
        printf("Node %d in component of root %d with size %d\n", i, conn_comps->parent[i],  conn_comps->size[i]);
    }*/

    return n_conn_comps;
}

void get_root(int *root_cc, int number_of_comps, Connected_component *conn_comps, int n_node) {
    char bitmap[n_node];
    for (int i = 0; i < n_node; i++) {
        bitmap[i] = 0;
    }
    //initialize the array
    for (int i = 0; i < number_of_comps; i++) {
        root_cc[i] = -1;
    }

    int t = 0;

    for (int i = 0; i < n_node; i++) {
        int cc = find(conn_comps, conn_comps->parent[i]);
        if (bitmap[cc] == 1) {
            continue;
        }
        bitmap[cc] = 1;
//        if (has_root(root_cc, cc, number_of_comps) != 0) {
//            continue;
//        }
        root_cc[t] = cc;
        t++;
    }
}

void free_comp(Connected_component *conn_comp) {
    free(conn_comp->parent);
    free(conn_comp->rank);
    free(conn_comp->size);
}