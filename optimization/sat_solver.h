/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef sat_solver_h
#define sat_solver_h

#include <vector>
#include <string>
#include <cassert>
#include <random>
#include <iostream>
#include "cnf.h"

const double MIN_SOLVING_TIME = 0.01;

using namespace std;

enum SOLVER_OUTPUT{ UNSAT = 0, SAT = 1, INTERR = 2 };

struct result {
    SOLVER_OUTPUT status;
	double runtime;
    void print() {
        cout << "status : " << status << endl;
        cout << "runtime : " << runtime << endl;
    }
};

class SatSolver
{
public:
    SatSolver(const string solver_name, const CNF cnf, const unsigned cpu_num,
          const unsigned sample_size, const unsigned incr_vars_num) : 
        solver_name(solver_name),
        cnf(cnf),
        cpu_num(cpu_num),
        sample_size(sample_size),
        incr_vars_num(incr_vars_num)
    {
        assert(solver_name != "");
        assert(cnf.var_num > 0 and cnf.clause_num > 0);
        assert(cpu_num >= 1);
        assert(sample_size > 0);
        assert(incr_vars_num > 0);
        assert(cnf_time_lim > 0);
    }
    double estimate(const vector<unsigned> point, mt19937 &rand_engine, const double cnf_time_lim);
    result solve(const string cnf_name, const double cnf_time_lim);
private:
    string solver_name;
    CNF cnf;
    unsigned cpu_num;
    unsigned sample_size;
    unsigned incr_vars_num;
    double cnf_time_lim;
};

#endif
