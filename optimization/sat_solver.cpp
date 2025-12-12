#include "sat_solver.h"
#include <random>
#include <iostream>

double SatSolver::estimate(const string cnf_name, const vector<unsigned> point_uint, mt19937 &rand_engine) {
    assert(not point_uint.empty());
    assert(cnf_name != "");
    assert(solver_name != "");
    assert(sample_size > 0);
    double res_estim = 0;
    // Calculate how many variables should be randomized:
    unsigned nonincrem_vars_num = point_uint.size() - incr_variables;
    assert(nonincrem_vars_num > 0);
    // Make a sample of random variables' values:
    uniform_int_distribution<int> dist(0, 1);
    vector<vector<bool>> sample_random_values(sample_size);
    for (unsigned i=0; i<sample_size; i++) {
        vector<bool> random_vec(nonincrem_vars_num);
        for (unsigned j=0; j<nonincrem_vars_num; j++) {
            bool random_bool = dist(rand_engine);
            random_vec[j] = random_bool;
        }
        sample_random_values[i] = random_vec;
    }
    // Process the random sample via a parallel loop:
    vector<double> runtimes(sample_size);
    for (unsigned i=0; i<sample_size; i++) {
        // For each set of random variables' values, make a CNF:
        //
        // Add unit clauses for values:
        //
        // Add incremental-like remaining variables:
        //  
        // Run an incremental SAT solver on the CNF:
        //
    }
    // Call an incremental SAT solver on the formed CNF:
    return res_estim;
}
