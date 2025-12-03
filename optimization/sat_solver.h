/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef sat_solver_h
#define sat_solver_h

#include <vector>
#include <string>
#include <cassert>

using namespace std;

class SatSolver
{
public:
    string solver_name;

    SatSolver(string solver_name) : solver_name(solver_name) {
        assert(solver_name != "");
    }

    double estimate(const string cnf_name, const vector<bool> point);
};

#endif
