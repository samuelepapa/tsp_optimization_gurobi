## Report topics

### Theory

**Description of the report**. A brief introduction.

**History of TSP**. Brief history, covering origin and what happened in the 20th century.

**Initial formulation**. Description of the formulation and why it's difficult to solve with exponential number of constraints (specify that the 2 forms of the SEC are mathematically the same, the have the same depth?).

**Compact models**. MTZ, exercise, flow1, flow2, flow3, ts1, ts2, ts3, cite the survey, online theoretical description, say that considerations have been done later.

**Solver-based methods**. Loop, lazy callback, user callback, theoretical description of why and how they should work. Description of the method used to find connected-components.

**Matheuristics**. Brief introduction describing the fact that they are a mix of heuristic and MIP solvers. Description of hard-fixing and local-branching.

**Metaheuristics**. 

*Construction*. Constructing solutions using nearest-neighbor, extra-mileage. Use of GRASP to add variability to the solution. The need for first-improvement tactic but anticipate the fact that they are extremely inefficient to apply to large isntances.

*Metaheuristics*. Introduction on metaheuristics, diversification and intensification.  GRASP as a metaheuristic. Variable Neighborhood Search general overview of the method and how it was used by us in general terms, use of 2-opt, say that things can be done to make it fast, k-opt neighborhood, the use of a 3-swap kick move, the double-bridge swap move (show the moves). Simulated annealing general description and how we decided to implement it (theoretical description of the implementation).

### Implementation

**Compact models**. Details on the use of lazy constraints. Effectiveness of flow1. High number of variables and constraints in other formulations. The apparent failure of the ts1, ts2, ts3 formulations.

**Solver-based methods**. Loop heuristics, cycling through the time to decrease the search of solutions which violate the constraints and how this makes it so useless constraints might be added. Lazy callback, the fact that Gurobi does everything from a single core, the necessity of removing pre-processor (which might affect performance). Usercut callback, the use of the Concorde library and the different formulation of the constraints used here.

**Matheuristics**. Look the the MF^2 paper.

**Metaheuristics**. Details on how the GRASP was implemented. Some examples of solutions found by pure greedy extra-mileage and pure greedy nearest neighbor. 2-opt implementation details. The choice of a simple data-structure over a more efficient one (cite the fact that there exists, but this report is based on an exploration of the different methods available not on a rigorous analysis and comparison of the best possible method available out there). The difference between using a random 3-opt kick and the double-bridge move to diversify and how effective they are because of the data structure chosen and how big the neighborhood becomes. Variable Neighborhood Search implementation details, the use of drag to let the neightborhood be explored for longer.

### Results

**Performance profiling**. Description of what performance profiling is.

**Profiling details**. How we decided to profile things: compact models, solver-based methods and metaheuristics.

**Results**. Display graphs and short summary tables. Comment on results after each section.

### Conclusions
Final conclusion on the experience? On the results obtained.

### Appendix
**Blade**. How to use university cluster Blade.

**Union-Find**. Explanation of the union-find algorithm.

?????**Iterative method**. Brief explanation of the iterative method for finding connected components.?????
