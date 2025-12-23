#include "sat_solver.h"
#include "utils.h"
#include <random>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>

#include <omp.h>

// Read SAT solver's result:
result readSolverResult(const string res_str) {
    assert(res_str != "");
	result res;
    res.status = INTERR;
    res.runtime = -1;
	stringstream sstream(res_str);
	string str;
	while (getline(sstream, str)) {
		if (str.find("s SATISFIABLE") != string::npos) {
			res.status = SAT;
		}
		else if (str.find("s UNSATISFIABLE") != string::npos) {
			res.status = UNSAT;
		}
        else if (str.find("% solve") != string::npos) {
            // Example: c         1.21   99.61% solve
			stringstream sstream(str);
            string first_word;
            sstream >> first_word; // skip the first word
            sstream >> res.runtime;
			break;
		}
	}
	return res;
}

result SatSolver::solve(const string cnf_name, const double cnf_time_lim) {
    assert(solver_name != "");
    assert(cnf_name != "");
    stringstream sstream;
    sstream << solver_name << " -t " << cnf_time_lim << " " << cnf_name;
    string system_str = sstream.str();
    //cout << system_str << endl;
    string res_str = utils::exec(system_str);
    result res = readSolverResult(res_str);
    // Exclude zero-runtime:
    if (res.runtime < MIN_SOLVING_TIME) res.runtime = MIN_SOLVING_TIME;
    return res;
}

double SatSolver::estimate(const vector<unsigned> point_uint, mt19937 &rand_engine, const double cnf_time_lim) {
    assert(not cnf.clauses.empty());
    assert(cnf.var_num > 0);
    assert(cnf.clause_num > 0);
    assert(not point_uint.empty());
    assert(solver_name != "");
    assert(sample_size > 0);
    double res_estim = 0;
    // Calculate how many variables should be randomized:
    unsigned nonincrem_vars_num = point_uint.size() - incr_vars_num;
    vector<vector<bool>> incr_vars_truth_table = utils::generateTruthTable(incr_vars_num);
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
    bool is_interrupted = false;
    vector<double> runtimes(sample_size);
    #pragma omp parallel for schedule(dynamic, 1)
    for (unsigned i=0; i<sample_size; i++) {
        //cout << "i " << i << endl;
        // For each set of random variables' values, make a CNF:
        stringstream sstream;
        sstream << "tmp_" << i << ".cnf";
        string tmp_cnf_name = sstream.str();
        ofstream tmp_cnf(tmp_cnf_name, ios_base::out);
        // Print header of the incremental DIMACS:
        tmp_cnf << "p inccnf " << endl;
        // Add main clauses: 
        for (auto &clause : cnf.clauses) tmp_cnf << clause << endl;
        // Add unit clauses for values:
        for (unsigned j=0; j<nonincrem_vars_num; j++) {
            if (not sample_random_values[i][j]) tmp_cnf << "-";
            tmp_cnf << point_uint[j] << " 0" << endl;
        }
        // Add cubes for incremental processing of the remaining variables:
        for (auto &bool_row : incr_vars_truth_table) {
            assert(bool_row.size() == incr_vars_num);
            tmp_cnf << "a ";
            for (unsigned j=0; j<incr_vars_num; j++) {
                if (not bool_row[j]) tmp_cnf << "-";
                tmp_cnf << point_uint[nonincrem_vars_num + j] << " ";
            }
            tmp_cnf << "0" << endl;
        }
        tmp_cnf.close();
        // Run an incremental SAT solver on the CNF:
        result res = solve(tmp_cnf_name, cnf_time_lim);
        string system_str = "rm -f " + tmp_cnf_name;
        utils::exec(system_str);
        // If at least one instance from the sample is not solved, no function value:
        if (res.status == INTERR) {
            is_interrupted = true;
        }
        runtimes[i] = res.runtime;
        //
    } // for
    // If at least one instance is not solved:
    if (is_interrupted) res_estim = -1;
    // If all instances from the sample are solved, func value is an average runtime * 2^nonincrem_vars_num:
    else {
        double sum_time = 0;
        for (auto &t : runtimes) sum_time += t;
        res_estim = (sum_time / sample_size) * (pow(2,nonincrem_vars_num));
    }
    
    return res_estim;
}
