#include "plot_graph.h"


void set_canvas(FILE *gnuplot_pipe);

void plot_instance(Tsp_prob *inst) {

    DEBUG_PRINT(("\n%s\n", "--start plot_instance method"));

    struct stat st = {0};

    if (stat("graph", &st) == -1) {
        mkdir("graph", 0700);
    }

    //file to store the points coordinates
    FILE *data = fopen("graph/data.dat", "w");

    for(int i = 0; i < inst->nnode; i++) {
        fprintf(data, "%lf %lf %d\n", inst->coord_x[i], inst->coord_y[i], i+1);
    }

    fclose(data);

    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE *gnuplot_pipe = popen("gnuplot -persistent", "w");

    //set teminal type with parameters, size in inch
    fprintf(gnuplot_pipe, "%s\n", "set terminal postscript portrait size 10, 8 \
enhanced color \"Helvetica\" 8");

    //output file
    fprintf(gnuplot_pipe, "%s\n", "set output 'graph/nodes.eps'");

    //set canvas
    set_canvas(gnuplot_pipe);

    //plot path with point style and label
    fprintf(gnuplot_pipe, "%s\n", "plot 'graph/data.dat' with labels point pointtype 7 offset char 1,-1.0 notitle");

    fprintf(gnuplot_pipe,"%s\n", "quit");

    pclose(gnuplot_pipe);

    remove("graph/data.dat");

    DEBUG_PRINT(("\n%s\n", "--plot completed"));

}
//TODO change solution format
/* First possible solution: make an array of couples of double these couples are interpreted as an edge
 *
 * */

/* old verision
void plot_edges(Tsp_prob *instance) {

    printf("\n%s\n", "--start plot_path method");

    //file to store the points coordinates of the path
    FILE *data = fopen("path.dat", "w");

    //number of nodes
    int n = instance->nnode;

    /*for(int i = 0; i < instance->num_nodes - 1; i++) {
        int node1 = instance->nodes_sequence[i];
        int node2 = instance->nodes_sequence[i+1];
        fprintf(data, "%lf %lf %d\n", instance->x_coord[node1], instance->y_coord[node1], node1);
        fprintf(data, "%lf %lf %d\n", instance->x_coord[node2], instance->y_coord[node2], node2);
        fprintf(data, "%s\n", "");

    }*/
/*
    //create path data file
    for(int i = 0; i < instance->solution_size; i++) {
        int node1 = instance->solution[i][0];
        int node2 = instance->solution[i][1];
        fprintf(data, "%lf %lf %d\n", instance->coord_x[node1-1], instance->coord_y[node1-1], node1);
        fprintf(data, "%lf %lf %d\n", instance->coord_x[node2-1], instance->coord_y[node2-1], node2);
        fprintf(data, "%s\n", "");

    }

    fclose(data);

    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
 /*   FILE *gnuplot_pipe = popen("gnuplot -persistent", "w");

    //set teminal type with parameters, size in inch
    fprintf(gnuplot_pipe, "%s\n", "set terminal postscript portrait size 10, 8 enhanced color \"Helvetica\" 6");

    //fprintf(gnuplot_pipe, "%s \n", "set size 1,1"); //set size of canvas

    //output file
    fprintf(gnuplot_pipe, "%s\n", "set output 'path.eps'");

    //fprintf(gnuplot_pipe, "%s\n", "set title 'TSP"); //plot title

    //fprintf(gnuplot_pipe, "%s\n", "set xlabel 'X'");

    //fprintf(gnuplot_pipe, "%s\n", "set ylabel 'Y'");

    //remove border
    fprintf(gnuplot_pipe, "%s\n", "unset border");

    //remove x-axis
    fprintf(gnuplot_pipe, "%s\n", "unset xtics");

    //remove y-axis
    fprintf(gnuplot_pipe, "%s\n", "unset ytics");

    //remove legend
    fprintf(gnuplot_pipe, "%s\n", "unset key");

    //fprintf(gnuplot_pipe, "%s\n", "set offset 1,1,1,1");

    //set style of the lines
    fprintf(gnuplot_pipe, "%s\n",  "set style line 1 \
									linecolor rgb '#0060ad' \
									linetype 1 linewidth 1" );

    //plot path with point style and label
    fprintf(gnuplot_pipe, "%s\n", "plot 'path.dat' with linespoints linestyle 1, '' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' notitle");

    pclose(gnuplot_pipe);

    printf("\n%s\n", "--plot completed");

}
*/

void plot_edges(Solution_list *edges_list, Tsp_prob * instance) {

    DEBUG_PRINT(("\n%s\n", "--start plot_edges function"));

     struct stat st = {0};

     if (stat("graph", &st) == -1) {
         mkdir("graph", 0700);
     }

    //file to store the points coordinates of the path
    FILE *data = fopen("graph/path.dat", "w");

    //FIRST SOLUTION
    for(int i = 0; i < edges_list->size; i++) {
        int node1 = edges_list->solution[i][0];
        int node2 = edges_list->solution[i][1];
        fprintf(data, "%lf %lf %d\n", instance->coord_x[node1], instance->coord_y[node1], node1+1);
        fprintf(data, "%lf %lf %d\n", instance->coord_x[node2], instance->coord_y[node2], node2+1);
        fprintf(data, "%s\n\n", "");

    }

    fclose(data);

     /*Opens an interface that one can use to send commands as if they were typing into the
      *     gnuplot command line.  "The -persistent" keeps the plot open even after your
      *     C program terminates.
      */
     FILE *gnuplot_pipe = popen("gnuplot -persistent", "w");

     //set teminal type with parameters, size in inch
     fprintf(gnuplot_pipe, "%s\n", "set terminal postscript portrait size 10, 8 enhanced color \"Helvetica\" 7");

     //output file
     fprintf(gnuplot_pipe, "%s\n", "set output 'graph/path.eps'");

     //set canvas
     set_canvas(gnuplot_pipe);

     //set style of the lines
     fprintf(gnuplot_pipe, "%s\n",  "set style line 1 \
									linecolor rgb '#0060ad' \
									linetype 1 linewidth 1" );

     char *model_name = calloc(64, 1);
     inverse_map_model_type(instance->model_type, model_name);

     fprintf(gnuplot_pipe, "set title \"%s using %s\"\n", instance->name, model_name);

     free(model_name);
     //plot path with point style and label
     fprintf(gnuplot_pipe, "%s\n", "plot 'graph/path.dat' with linespoints linestyle 1, '' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' notitle");

     fprintf(gnuplot_pipe,"%s\n", "quit");

     pclose(gnuplot_pipe);

     remove("graph/path.dat");

     DEBUG_PRINT(("\n%s\n", "--plot completed"));

}

void set_canvas(FILE *gnuplot_pipe) {

    //fprintf(gnuplot_pipe, "%s \n", "set size 1,1"); //set size of canvas

    //fprintf(gnuplot_pipe, "%s\n", "set title 'Graph nodes'"); //plot title

    //fprintf(gnuplot_pipe, "%s\n", "set xlabel 'X'");

    //fprintf(gnuplot_pipe, "%s\n", "set ylabel 'Y'");

    //remove border
    fprintf(gnuplot_pipe, "%s\n", "unset border");

    //remove x-axis
    fprintf(gnuplot_pipe, "%s\n", "unset xtics");

    //remove y-axis
    fprintf(gnuplot_pipe, "%s\n", "unset ytics");

    //remove legend
    fprintf(gnuplot_pipe, "%s\n", "unset key");

    //set margin
    fprintf(gnuplot_pipe, "%s\n", "set lmargin at screen 0.05");
    fprintf(gnuplot_pipe, "%s\n", "set rmargin at screen 0.95");
    fprintf(gnuplot_pipe, "%s\n", "set bmargin at screen 0.05");
    fprintf(gnuplot_pipe, "%s\n", "set tmargin at screen 0.95");


    //fprintf(gnuplot_pipe, "%s\n", "set offset 1,1,1,1");
}

