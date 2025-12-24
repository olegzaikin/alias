/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef base_ls_h
#define base_ls_h

#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <set>
#include <cmath>
#include <random>
#include <unordered_map>
#include <cassert>
#include "point.h"
#include "cnf.h"

const double DEFAULT_WALL_TIME_LIMIT = 86400;
const double DEFAULT_CNF_TIME_LIMIT = 600;
const unsigned MAX_SOLVING_VARS = 48;
const unsigned DEFAULT_JUMP_LIM = 3;
const unsigned MIN_VARS_JUMP = 100;
const double MAX_OBJ_FUNC_VALUE = 1e+300;

#define SOLVE 1
#define ESTIMATE 2

using namespace std;

class base_local_search
{
public:
	base_local_search();
	void parseParams(vector<string> str_argv);
	void init();

protected:
	int getCpuCores();
	unsigned elapsedWallTime();
	bool isTimeExceeded();
	Point pointFromUintVec(vector<unsigned> var_vec);
	vector<unsigned> uintVecFromPoint(Point p);
	void printUintVec(const vector<unsigned> vec);
	void printBoolVec(const vector<bool> vec);
	vector<Var> getAllCnfVars(const string filename);
	vector<Var> readVarsFromPcs(string pcs_name);
	bool isEstTooLong();
	void writeToGraphFile(const string str);
	void setGraphFileName();
	void reportOptResult();
	void calculateEstimation(Point &cur_point, bool use_memory = true);
	bool isChecked(Point p);
	void clearInterruptedChecked();
	int getVarPos(const unsigned val);
	void printGlobalRecordPoint();
	string printUintVector(vector<unsigned>);
	int total_func_calculations;
	int total_skipped_func_calculations;
	int total_interr_func_calculations;
	// Input variables:
	unsigned opt_alg;
	string cnf_name;
	string solver_name;
	string pcs_name;
	int cpu_num;
	double cnf_time_lim;
	double wall_time_lim;
	unsigned sample_size;
	unsigned incr_vars_num;
	unsigned seed;
	int verbosity;
	// Internal variables:
	fstream graph_file;
	string graph_file_name;
	vector<Var> vars; // all vars in the search space
	bool are_vars_in_row;
	unordered_map<string, double> checked_points;
	unsigned skipped_points_count;
	unsigned interrupted_points_count;
	Point global_record_point;
	Point local_record_point;
	Point before_jump_point;
	bool is_random_search;
	unsigned vars_decr_times;
	chrono::high_resolution_clock::time_point start_time;
	bool is_solve;
	string result_output_name;
	string backdoor_file_name;
	mt19937 rand_engine;
	// iteretedHCVJ parameters
	unsigned jump_lim;
	unsigned jump_step;
	bool is_jump_mode;
	CNF orig_cnf;
	Point known_backdoor;
	void loadVars();
	void loadBackdoor();
	void solveInstance();
	Point getStartPoint();
};

#endif
