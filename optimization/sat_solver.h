/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef sat_solver_h
#define sat_solver_h

#include <vector>
#include <string>
#include <cassert>
#include <random>

using namespace std;

class SatSolver
{
public:
    string solver_name;
    unsigned sample_size;
    unsigned incr_variables;

    SatSolver(string solver_name, unsigned sample_size, unsigned incr_variables) : 
        solver_name(solver_name),
        sample_size(sample_size),
        incr_variables(incr_variables)
    {
        assert(solver_name != "");
    }

    double estimate(const string cnf_name, const vector<unsigned> point, mt19937 &rand_engine);
};

#endif
