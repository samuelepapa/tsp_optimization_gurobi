// Gnuplot/C interface library header file
// Please ensure that the system path includes an entry for the gnuplot binary folder

#ifndef TSPOPT_GPC_H

#define TSPOPT_GPC_H 1                   // Gnuplot/C included

#define TSPOPT_GPC_DEBUG

#define CANVAS_WIDTH 1920
#define CANVAS_HEIGHT 1080

typedef struct                                      // Complex data type
{
    double  x;
    double  y;
} Points;


typedef struct
{
    char filename [40];                             // Graph filename
    char title [80];                                // Graph title
    char formatString [40];                         // Graph format string "lines", "points" etc
} h_GPC_Graph;


typedef struct
{
    FILE *pipe;                                     // Pipe to Gnuplot instance
    char plotTitle[80];                             // Plot title
    double xMin;                                    // Minimum value of X axis - used for axis labels
    double xMax;                                    // Maximum value of X axis - used for axis labels
    double yMin;                                    // Minimum value of Y axis - used for axis labels
    double yMax;                                    // Maximum value of Y axis - used for axis labels
} h_GPC_Plot;

h_GPC_Plot *gpc_init_xy(const char *plotTitle,
                        const double xmin, const double ymin, const double xmax, const double ymax);                 // Legend / key mode
    
int gpc_plot_xy (h_GPC_Plot *plotHandle,            // Plot handle
    const Points *pData,                     // Dataset pointer
    const int pLength,                          // Dataset length
    const char *pDataName,                          // Dataset title
    const char *plotType,                           // Plot type - "lines", "points", "impulses", "linespoints"
    const char *pColour
    );

void gpc_close (h_GPC_Plot *plotHandle);            // Plot handle

#endif    // End of GPC_H
