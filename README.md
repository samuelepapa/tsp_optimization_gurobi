# tsp_optimization_gurobi

This is the repository of the code written during the *Operations Research 2* course, taught by Matteo Fischetti at 
the University of Padova during the 2018/2019 academic year.

We study how to find an optimal solution to the Travelling Salesman problem using state-of-the-art software.

### Style guidelines
Underscore to name variable `name_of_variable`.
 
Structs with `Uppercase_underscore_style`.

Every library should have a header where functions usable by other executables are declared. Functions used inside the 
same file can be declared and used there directly

[C GNU styleguide](https://www.gnu.org/prep/standards/html_node/Writing-C.html)

### Model definition

First define the environment, the model. Then start by adding all variables, specifying the 
necessary variables (define everything in arrays), add then the constraints. Print the model
on file.

Second, optimize the model, here maybe parameters can be changed, the model is ready, 
so isolation is achieved.

Third, plot solution found. Investigate whether solution can be plotted while it is being found.

### TODO
* Make plot_instance work using call to GRB function (use tolerance).
* Define level of verbosity for logging and take it as input.
* Make code work with more types of files of input.
* Define help text for command line.
* Fix folder structure.
* Labels on edges.