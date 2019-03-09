//
// Created by samuele on 05/03/19.
//

#include "common.h"
#include "utils.h"
#include "tsp.h"
#include "plotGraph.h"
#include "utils.c"
#include "inputOutput.c"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Not enough arguments.\n");
        exit(1);
    }

    Tsp_prob instance = {
            .nnode = -1
    };
    //GRBerror message.
    int valid_instance = 0;

    parse_input(argc, argv, &instance);

    valid_instance = init_instance(&instance);

    if (valid_instance) {

        plot_instance(&instance);

        close_instance(&instance);

    }else{
        printf("Error in parsing file");
        exit(1);
    }
    return 0;
}


/*void plot_instance(Tsp_prob *instance) {
    Points *points;
    GPC_Plot *plot1;
    points = calloc((size_t) instance->nnode, sizeof(GPC_Plot));

    // to display all points in graph
    double smallest_x = instance->coord_x[0];
    double smallest_y = instance->coord_y[0];
    double biggest_x = instance->coord_x[0];
    double biggest_y = instance->coord_y[0];

    //populate points
    for (int i = 0; i < instance->nnode; i++) {
        points[i].x = instance->coord_x[i];
        points[i].y = instance->coord_y[i];
        // find edges
        if (points[i].x < smallest_x) smallest_x = points[i].x;
        if (points[i].x > biggest_x) biggest_x = points[i].x;
        if (points[i].y < smallest_y) smallest_y = points[i].y;
        if (points[i].y > biggest_y) biggest_y = points[i].y;
    }

    plot1 = gpc_init_xy("Plot of points", smallest_x - 200, smallest_y - 200, biggest_x + 200, biggest_y + 200);
    gpc_plot_xy(plot1, points, instance->nnode, "points", "circles", "blue");
    plot_line(instance, plot1, 0, 1);
    plot_line(instance, plot1, 10, 11);

    plot_instance(instance);
    //plot_path(instance);
}*/