#include "sat_solver.h"

double SatSolver::estimate(const string cnf_name, const vector<bool> point) {
    assert(not point.empty());
    assert(cnf_name != "");
    assert(solver_name != "");
    double res_estim = 0;
    // Make a random sample of variables' values:
    //
    // Process the random sample via a parallel loop:
    // 
    // For each set of variables' values, make a CNF:
    //
    // Call an incremental SAT solver on the formed CNF:
    return res_estim;
}
