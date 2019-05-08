# tsp_optimization_gurobi

This is the repository of the code written during the *Operations Research 2* course, taught by Matteo Fischetti at 
the University of Padova during the 2018/2019 academic year.

We study how to find an optimal solution to the Travelling Salesman problem using state-of-the-art software.

### Run/Debug configurations
In the program arguments of tsp_optimize configuration add -f ALL_tsp/name_of_tsp_data_file.tsp 
### Style guidelines
Underscore to name variable `name_of_variable`.
 
Structs with `Uppercase_underscore_style`.

Every library should have a header where functions usable by other executables are declared. Functions used inside the 
same file can be declared and used there directly

[C GNU styleguide](https://www.gnu.org/prep/standards/html_node/Writing-C.html)

The functions used to find the position in the model should be name `xpos_nameofmodel` where x 
is meant to be the name of the set of variable this function refers to (this to avoid error 
about redefining functions).

### Model definition

First define the environment, the model. Then start by adding all variables, specifying the 
necessary variables (define everything in arrays), add then the constraints. Print the model
on file.

Second, optimize the model, here maybe parameters can be changed, the model is ready, 
so isolation is achieved.

Third, plot solution found. Investigate whether solution can be plotted while it is being found.

### Heuristics
Heuristics work by starting from a non-optimal solution to our problem and then finding a new 
solution using some technique. Matheuristics define a neighbourhood of the solution and they 
explore it iteratively, since most of the variables are set, the solver will be very fast, and
a new solution defined, from which a new neighbourhood can be set and the process repeated.

To do this, loop, lazycall and usercall are split in generation and creation.


### TODO


