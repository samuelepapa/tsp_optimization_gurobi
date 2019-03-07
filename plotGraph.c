//
// Created by davide on 07/03/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"

void plot_nodes(Tsp_prob *inst) {

    printf("\n%s\n", "--start plot_nodes method");

    FILE *data = fopen("data.dat", "w");

    for(int i = 0; i < inst->nnode; i++) {
        fprintf(data, "%lf %lf %d\n", inst->coord_x[i], inst->coord_y[i], i+1);
    }

    fclose(data);

    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE *gnuplot_pipe = popen("gnuplot -persistent", "w");

    fprintf(gnuplot_pipe, "%s\n", "set terminal postscript portrait size 10, 8 \
enhanced color \"Helvetica\" 8"); //set teminal type with parameters, size in inch

    //fprintf(gnuplot_pipe, "%s \n", "set size 1,1"); //set size of canvas

    fprintf(gnuplot_pipe, "%s\n", "set output 'nodes.eps'"); //output file

    //fprintf(gnuplot_pipe, "%s\n", "set title 'Graph nodes'"); //plot title

    //fprintf(gnuplot_pipe, "%s\n", "set xlabel 'X'");

    //fprintf(gnuplot_pipe, "%s\n", "set ylabel 'Y'");

    fprintf(gnuplot_pipe, "%s\n", "unset border"); //remove border

    fprintf(gnuplot_pipe, "%s\n", "unset xtics"); //remove x axes

    fprintf(gnuplot_pipe, "%s\n", "unset ytics"); //remove y axes

    //fprintf(gnuplot_pipe, "%s\n", "set offset 1,1,1,1");

    fprintf(gnuplot_pipe, "%s\n", "plot 'data.dat' with labels point pointtype 7 offset char 1,-1.0 notitle");

    pclose(gnuplot_pipe);

    printf("\n%s\n", "--plot completed");

}

void plot_path(Tsp_prob *inst) {

    printf("\n%s\n", "--start plot_path method");

    FILE *data = fopen("path.dat", "w");

    int n = inst->nnode;

    /*for(int i = 0; i < inst->num_nodes - 1; i++) {
        int node1 = inst->nodes_sequence[i];
        int node2 = inst->nodes_sequence[i+1];
        fprintf(data, "%lf %lf %d\n", inst->x_coord[node1], inst->y_coord[node1], node1);
        fprintf(data, "%lf %lf %d\n", inst->x_coord[node2], inst->y_coord[node2], node2);
        fprintf(data, "%s\n", "");

    }*/

    for(int i = 0; i < n; i++) { //create edge data file
        int node1 = inst->solution[i%n];
        int node2 = inst->solution[(i+1)%n];
        fprintf(data, "%lf %lf %d\n", inst->coord_x[node1-1], inst->coord_y[node1-1], node1);
        fprintf(data, "%lf %lf %d\n", inst->coord_x[node2-1], inst->coord_y[node2-1], node2);
        fprintf(data, "%s\n", "");

    }

    fclose(data);

    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE *gnuplot_pipe = popen("gnuplot -persistent", "w");

    fprintf(gnuplot_pipe, "%s\n", "set terminal postscript portrait size 10, 8 \
enhanced color \"Helvetica\" 6"); //set teminal type with parameters, size in inch

    //fprintf(gnuplot_pipe, "%s \n", "set size 1,1"); //set size of canvas

    fprintf(gnuplot_pipe, "%s\n", "set output 'path.eps'"); //output file

    //fprintf(gnuplot_pipe, "%s\n", "set title 'Graph nodes'"); //plot title

    //fprintf(gnuplot_pipe, "%s\n", "set xlabel 'X'");

    //fprintf(gnuplot_pipe, "%s\n", "set ylabel 'Y'");

    fprintf(gnuplot_pipe, "%s\n", "unset border");

    fprintf(gnuplot_pipe, "%s\n", "unset xtics");

    fprintf(gnuplot_pipe, "%s\n", "unset ytics");

    fprintf(gnuplot_pipe, "%s\n", "unset key");

    //fprintf(gnuplot_pipe, "%s\n", "set offset 1,1,1,1");

    fprintf(gnuplot_pipe, "%s\n",  "set style line 1 \
									linecolor rgb '#0060ad' \
									linetype 1 linewidth 1" );

    fprintf(gnuplot_pipe, "%s\n", "plot 'path.dat' with linespoints linestyle 1, '' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' notitle");

    pclose(gnuplot_pipe);

    printf("\n%s\n", "--plot completed");


}

