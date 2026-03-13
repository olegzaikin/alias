#include "base_ls.h"
#include "sat_solver.h"
#include "utils.h"
#include <sstream>
#include <omp.h>

base_local_search::base_local_search() :
	graph_file_name(""),
	cnf_name(""),
	solver_name(""),
	pcs_name(""),
	cpu_num(0),
	wall_time_lim(DEFAULT_WALL_TIME_LIMIT),
	cnf_time_lim(DEFAULT_CNF_TIME_LIMIT),
	skipped_points_count(0),
	interrupted_points_count(0),
	is_jump_mode(true),
	vars_decr_times(0),
	is_solve(false),
	jump_lim(DEFAULT_JUMP_LIM),
	result_output_name(""),
	opt_alg(5), // 1+1
	total_func_calculations(0),
	total_skipped_func_calculations(0),
	total_interr_func_calculations(0),
	sample_size(100),
	incr_vars_num(7),
	seed(0),
	verbosity(0),
	are_vars_in_row(false)
{
	start_time = chrono::high_resolution_clock::now(); 
	// Initialize a random generator by the given seed:
	global_record_point.value.resize(0);
	local_record_point.estimation = HUGE_VAL;
	global_record_point.estimation = HUGE_VAL;
}

void base_local_search::loadVars()
{
	if (pcs_name != "")
		vars = readVarsFromPcs(pcs_name); // get search space from a given pcs-file
	else
		vars = getAllCnfVars(cnf_name);

	if (vars.size() < MIN_VARS_JUMP) {
		cout << "small search space, turn off the jump mode\n";
		is_jump_mode = false;
	}

	if (!vars.size()) {
		cerr << "*** search space is empty" << endl;
		exit(-1);
	}

	cout << "decomposition set size : " << vars.size() << endl;
	for (auto v : vars)
		cout << v.value << " ";
	cout << endl;
	
	are_vars_in_row = true;
	for (unsigned i = 0; i < vars.size(); i++) {
		if (vars[i].value != i+1) {
			are_vars_in_row = false;
			break;
		}
	}
}

// Load a backdoor from a file.
// It is written in one line as integers divided by spaces: 
void base_local_search::loadBackdoor()
{
	if (backdoor_file_name != "") {
		if (verbosity > 1)
			cout << "loading a backdoor from file " << backdoor_file_name << endl;
		ifstream ifile(backdoor_file_name);
		stringstream sstream;
		string str;
		getline(ifile, str);
		sstream << str;
		vector<unsigned> var_vec;
		int val;
		while (sstream >> val)
			var_vec.push_back(val);
		ifile.close();
		known_backdoor = pointFromUintVec(var_vec);
		cout << "known backdoor size : " << var_vec.size() << endl;
		cout << "known backdoor : " << endl;
		printUintVec(var_vec);
		/*
		// calculate estimation for the given backdoor
		calculateEstimation(known_backdoor);
		global_record_point = known_backdoor;
		printGlobalRecordPoint();
		*/
	}
}

int base_local_search::getVarPos(const unsigned val)
{
	int pos = -1;
	if (are_vars_in_row) { // don't find a position, variables are in a row
		pos = val - 1;
	}
	else { // find a position
		for (int i = 0; i < vars.size(); i++)
			if (vars[i].value == val) {
				pos = i;
				break;
			}
	}
	if (pos == -1) {
		cerr << "pos == -1" << endl;
		cerr << "val : " << val << endl;
		exit(-1);
	}
	return pos;
}

Point base_local_search::pointFromUintVec(vector<unsigned> var_vec)
{
	Point p;
	p.value.resize(vars.size());
	for (auto x : p.value)
		x = false;
	for (auto val : var_vec) {
		int pos = getVarPos(val);
		p.value[pos] = true;
	}
	return p;
}

vector<unsigned> base_local_search::uintVecFromPoint(Point p)
{
	vector<unsigned> vec;
	for (unsigned i = 0; i < p.value.size(); i++)
		if (p.value[i])
			vec.push_back(vars[i].value);
	return vec;
}

void base_local_search::printUintVec(const vector<unsigned> vec)
{
	for (auto x : vec)
		cout << x << " ";
	cout << endl;
}

void base_local_search::printBoolVec(const vector<bool> vec)
{
	for (auto x : vec)
		cout << x << " ";
	cout << endl;
}

// Parse a dimacs CNF formula from a given file and
// save its clauses into a given vector.
vector<Var> base_local_search::getAllCnfVars(const string filename)
{
	int vars_count = 0;
	ifstream ifile(filename.c_str());
	if (!ifile.is_open()) {
		cerr << "Error : failed to open " << filename << endl;
		exit(-1);
	}

	string str;

	while (getline(ifile, str)) {
		if ((!str.size()) || (str[0] == 'c'))
			continue; // skip empty and comment strings
		stringstream sstream;
		sstream << str;
		if (str[0] == 'p') {
			cout << "try to get vars from string " << str << endl;
			sstream >> str >> str >> vars_count;
		}
	}

	if (vars_count <= 0) {
		cerr << "*** incorrect vars_count " << vars_count << endl;
		exit(-1);
	}

	ifile.close();

	vector<Var> vars_vec;
	Var tmp_var;
	for (int i = 0; i < vars_count; i++) {
		tmp_var.value = i + 1;
		tmp_var.calculations = 0;
		tmp_var.global_records = 0;
		tmp_var.obj_val_remove = MAX_OBJ_FUNC_VALUE;
		tmp_var.obj_val_add = MAX_OBJ_FUNC_VALUE;
		vars_vec.push_back(tmp_var);
	}

	return vars_vec;
}

vector<Var> base_local_search::readVarsFromPcs(const string pcs_name)
{
	ifstream pcs_file(pcs_name);
	if (!pcs_file.is_open()) {
		cerr << "error while opening " << pcs_name;
		exit(-1);
	}

	string str;
	vector<Var> vars_vec;
	Var tmp_var;
	while (getline(pcs_file, str)) {
		if (str.size() <= 2)
			continue;
		size_t pos = str.find(' ');
		if (pos != string::npos)
			str = str.substr(1, pos - 1);
		bool isDigitVal = true;
		for (auto &x : str)
			if (!isdigit(x)) {
				isDigitVal = false;
				break;
			}
		if (!isDigitVal)
			continue;
		stringstream sstream;
		sstream << str;
		sstream >> tmp_var.value;
		tmp_var.calculations = 0;
		tmp_var.global_records = 0;
		tmp_var.obj_val_remove = MAX_OBJ_FUNC_VALUE;
		tmp_var.obj_val_add = MAX_OBJ_FUNC_VALUE;
		vars_vec.push_back(tmp_var);
	}
	
	pcs_file.close();

	return vars_vec;
}

void base_local_search::setGraphFileName()
{
	string cnf_name_short, solver_name_short;
	size_t found1 = cnf_name.find_last_of("/");
	size_t found2 = cnf_name.find_last_of(".");
	if (found1 != string::npos)
		cnf_name_short = cnf_name.substr(found1+1, found2-found1-1);
	else
		cnf_name_short = cnf_name;
	found1 = solver_name.find_last_of("/");
	if (found1 != string::npos)
		solver_name_short = solver_name.substr(found1 + 1, solver_name.size() - found1 - 1);
	else
		solver_name_short = solver_name;
	cout << "cnf_name_short " << cnf_name_short << endl;
	graph_file_name = "alias_" + solver_name_short + "_" + cnf_name_short;
	cout << "graph_file_name " << graph_file_name << endl;
	graph_file.open(graph_file_name, ios_base::out); // erase file
	graph_file.close();
}

void base_local_search::writeToGraphFile(const string str) 
{
	graph_file.open(graph_file_name, ios_base::app);
	graph_file << str << endl;
	graph_file.close();
}

int base_local_search::getCpuCores()
{
	int cpu_num = std::thread::hardware_concurrency();
	//cout << "cpu_num " << cpu_num << endl;
	if (cpu_num <= 0)
		exit(-1);
	return cpu_num;
}

string base_local_search::printUintVector(vector<unsigned> vec)
{
	stringstream sstream;
	for (auto x : vec)
		sstream << x << " ";
	return sstream.str();
}

unsigned base_local_search::elapsedWallTime()
{
	chrono::high_resolution_clock::time_point cur_time = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_time - start_time);
	return time_span.count();
}

bool base_local_search::isTimeExceeded()
{
	chrono::high_resolution_clock::time_point cur_time = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_time - start_time);
	if (time_span.count() >= wall_time_lim) {
		cout << "*** time is up" << endl;
		return true;
	}
	return false;
}

bool base_local_search::isEstTooLong() // for simple instances
{
	if ((global_record_point.estimation / cpu_num / 2) <= elapsedWallTime()) {
		// additionally divide by 2 for possible satisfiable instances
		cout << "*** estimation / " << cpu_num << " and /2 is less than elapsed time" << endl;
		return true;
	}
	return false;
}

void base_local_search::reportOptResult()
{
	stringstream sstream;
	sstream << "Function calculations : " << total_func_calculations << endl;
	sstream << "Interrupted function calculations : " << total_interr_func_calculations << endl;
	sstream << "Skipped function calculations : " << total_skipped_func_calculations << endl;
	sstream << "Elapsed wall time : " << elapsedWallTime() << endl;
	sstream << "Backdoor size : " << global_record_point.weight() << endl;
	sstream << "Backdoor (numeration from 1) :" << endl;
	sstream << global_record_point.getStr(vars);
	sstream << "Estimation for 1 CPU core : " << global_record_point.estimation << " seconds" << endl;
	sstream << "Estimation for " << cpu_num << " CPU cores : " << global_record_point.estimation / cpu_num << " seconds" << endl;
	cout << sstream.str();
	
	if (result_output_name != "") {
		ofstream ofile(result_output_name);
		ofile << sstream.str();
		ofile.close();
	}
}

bool strPrefix(const string init_str, const string prefix, string &res_str)
{
	size_t found = init_str.find(prefix);
	if (found != string::npos) {
		res_str = init_str.substr(found + prefix.length());
		return true;
	}
	return false;
}

void base_local_search::parseParams(vector<string> str_argv)
{
	assert(str_argv.size() >= 4);
	cnf_name = str_argv[1];
	pcs_name = str_argv[2];
	solver_name = str_argv[3];
	for (auto &par_str : str_argv) {
		string res_str;
		if (strPrefix(par_str, "-solver=", res_str))
			solver_name = res_str;
		else if (strPrefix(par_str, "-optalg=", res_str))
			istringstream(res_str) >> opt_alg;
		else if (strPrefix(par_str, "-backdoor=", res_str))
			istringstream(res_str) >> backdoor_file_name;
		else if (strPrefix(par_str, "-sample=", res_str))
			istringstream(res_str) >> sample_size;
		else if (strPrefix(par_str, "-walltimelim=", res_str))
			istringstream(res_str) >> wall_time_lim;
		else if (strPrefix(par_str, "-cnftimelim=", res_str))
			istringstream(res_str) >> cnf_time_lim;
		else if (strPrefix(par_str, "-cpunum=", res_str))
			istringstream(res_str) >> cpu_num;
		else if (strPrefix(par_str, "-verb=", res_str))
			istringstream(res_str) >> verbosity;
		else if (strPrefix(par_str, "-seed=", res_str))
			istringstream(res_str) >> seed;
		else if (par_str == "--solve")
			is_solve = true;
	}

	if (cpu_num == 0) cpu_num = getCpuCores();
	
	cout << "cnf name " << cnf_name << endl;
	cout << "decomposition set name " << pcs_name << endl;
	cout << "solver name " << solver_name << endl;
	cout << "is solve " << is_solve << endl;
	if (is_solve) {
		cout << "backdoor_file_name " << backdoor_file_name << endl;
	}
	else {
		cout << "opt_alg " << opt_alg << endl;
		cout << "sample_size " << sample_size << endl;
	}
	cout << "wall_time_lim " << wall_time_lim << endl;
	cout << "CPU num " << cpu_num << endl;
	cout << "CNF time limit " << cnf_time_lim << endl;
	cout << "verbosity " << verbosity << endl;
	cout << "seed " << seed << endl;

	string base_cnf_name = cnf_name;
	size_t pos = cnf_name.find_last_of("/");
	if (pos != string::npos) {
		base_cnf_name = cnf_name.substr(pos+1, cnf_name.size() - pos - 1);
	}
	result_output_name = "out_" + base_cnf_name;
	cout << "result_output_name : " << result_output_name << endl; 
	orig_cnf.read(cnf_name);
	assert(cnf_name != "");
}

void base_local_search::init()
{
	rand_engine.seed(seed);
	omp_set_num_threads(cpu_num);
	loadVars();
	loadBackdoor();
	setGraphFileName();
}

void base_local_search::clearInterruptedChecked()
{
	cout << "clearing interrupted points \n";
	cout << checked_points.size() << " points before\n";
	for (unordered_map<string, double>::iterator it = checked_points.begin();
		it != checked_points.end();)
	{
		if (it->second >= MAX_OBJ_FUNC_VALUE)
			it = checked_points.erase(it);
		else
			++it;
	}
	cout << checked_points.size() << " points after\n";
}

void base_local_search::calculateEstimation(Point &cur_point, bool use_memory)
{
	string str = "";
	// If a point has been already checked, take the estimatation from the map:
	if (use_memory) {
		// don't use isChecked since an iterator is required here
		for (auto x : cur_point.value)
			str += x == true ? '1' : '0';
		unordered_map<string, double>::iterator it = checked_points.find(str);
		if (it != checked_points.end()) {
			cur_point.estimation = it->second;
			total_skipped_func_calculations++;
			return;
		}
	}
	
	vector<unsigned> point_uint = uintVecFromPoint(cur_point);
	SatSolver solver(solver_name, orig_cnf, cpu_num, sample_size, incr_vars_num);
	cur_point.estimation = solver.estimate(point_uint, rand_engine, cnf_time_lim);
	if (cur_point.estimation == -1) total_interr_func_calculations++;

	// Save the point if memory is being used:
	if (use_memory) {
		checked_points.insert(pair<string, double>(str, cur_point.estimation));
	}
	if ((!is_jump_mode) && (!is_random_search)) {
		for (unsigned j = 0; j < cur_point.value.size(); j++)
			if (cur_point.value[j])
				vars[j].calculations++;
	}
	total_func_calculations++;
}

bool base_local_search::isChecked(Point p)
{
	string str = "";
	for (auto x : p.value)
		str += x == true ? '1' : '0';
	if (checked_points.find(str) != checked_points.end())
		return true;
	return false;
}

void base_local_search::printGlobalRecordPoint()
{
	cout << "point weight : " << global_record_point.weight() << endl;
	cout << "runtime estimation on 1 CPU core : " << global_record_point.estimation << endl;
	cout << "runtime estimation on " << cpu_num << " CPU num : " << global_record_point.estimation / cpu_num << endl;
	cout << "backdoor : " << endl;
	cout << global_record_point.getStr(vars);
}

void print_solve_stats(const unsigned processed_tasks,
	                   const unsigned total_tasks,
					   const unsigned unsat_num,
	                   const unsigned sat_num,
					   const unsigned interr_num) {
	cout << "processed " << processed_tasks << " out of " << total_tasks << "  ";
	cout << "unsat : " << unsat_num << " ";
	cout << "sat : " << sat_num << " ";
	cout << "interr : " << interr_num;
	cout << endl;
}

Point base_local_search::getStartPoint() {
	Point start_point;
	start_point.value.resize(vars.size());
	for (auto x : start_point.value)
		x = true;
	return start_point;
}

void base_local_search::solveInstance()
{
	assert(is_solve);
	cout << "solve the instance using a decomposition set" << endl;

	stringstream sstream;
	sstream << "--- start solving, elapsed time " << elapsedWallTime();
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();

	double remaining_wall_time = wall_time_lim - elapsedWallTime();
	cout << "remaining_wall_time : " << remaining_wall_time << endl;

	assert(cpu_num > 0);

	// assert(vars.size() <= MAX_SOLVING_VARS);

	assert(not known_backdoor.value.empty());
	assert(known_backdoor.weight() > 0 and known_backdoor.weight() <= vars.size());
	assert(known_backdoor.value.size() == vars.size());
	vector<unsigned> point_uint = uintVecFromPoint(known_backdoor);
	unsigned vars_to_vary = point_uint.size() - incr_vars_num;

	vector<vector<bool>> vary_vars_truth_table = utils::generateTruthTable(vars_to_vary);
	vector<vector<bool>> incr_vars_truth_table = utils::generateTruthTable(incr_vars_num);

	SatSolver solver(solver_name, orig_cnf, cpu_num, sample_size, incr_vars_num);
	unsigned unsat_num = 0;
	unsigned sat_num = 0;
	unsigned interr_num = 0;
	unsigned processed = 0;
	string system_str;

	#pragma omp parallel for schedule(dynamic, 1)
	for (unsigned i=0; i<vary_vars_truth_table.size(); i++) {
		assert(vary_vars_truth_table[i].size() == vars_to_vary);
		remaining_wall_time = wall_time_lim - elapsedWallTime();
		// If SAT found or time limit is reached, skip all unsolved instances:
		if ( (sat_num > 0) or (isTimeExceeded()) ) {
			continue;
		}
		// For each set of random variables' values, make a CNF:
        stringstream sstream;
        sstream << "tmp_" << i << ".cnf";
        string tmp_cnf_name = sstream.str();
        ofstream tmp_cnf(tmp_cnf_name, ios_base::out);
        // Print header of the incremental DIMACS:
        tmp_cnf << "p inccnf " << endl;
        // Add main clauses: 
        for (auto &clause : orig_cnf.clauses) tmp_cnf << clause << endl;
        // Add unit clauses for varied variables:
        for (unsigned j=0; j<vary_vars_truth_table[i].size(); j++) {
            if (not vary_vars_truth_table[i][j]) tmp_cnf << "-";
            tmp_cnf << point_uint[j] << " 0" << endl;
        }
        // Add cubes for incremental processing of the remaining variables:
        for (auto &bool_row : incr_vars_truth_table) {
            assert(bool_row.size() == incr_vars_num);
            tmp_cnf << "a ";
            for (unsigned j=0; j<incr_vars_num; j++) {
                if (not bool_row[j]) tmp_cnf << "-";
                tmp_cnf << point_uint[vars_to_vary + j] << " ";
            }
            tmp_cnf << "0" << endl;
        }
        tmp_cnf.close();
        // Run an incremental SAT solver on the CNF:
        result res = solver.solve(tmp_cnf_name, remaining_wall_time);
		if (res.status == SAT) {
			sat_num++;
			// Make a copy of a satisfiable CNF: 
			stringstream sstream_sat_cnf_name;
			sstream_sat_cnf_name << "!sat_cnf_i=" << i << ".cnf";
			string sat_cnf_name = sstream_sat_cnf_name.str();
			system_str = "cp " + tmp_cnf_name + " " + sat_cnf_name;
			utils::exec(system_str);
			cout << "SAT found on i " << i << endl;
			cout << "SAT CNF written to " << sat_cnf_name << endl;
		}
		else {
			assert(res.status == UNSAT or res.status == INTERR);
			if (res.status == UNSAT) unsat_num++;
			if (res.status == INTERR) interr_num++;
		}
		system_str = "rm -f " + tmp_cnf_name;
		utils::exec(system_str);
		processed++;
		if (processed > 1 and processed % 100 == 0) {
			print_solve_stats(processed, vary_vars_truth_table.size(), unsat_num, sat_num, interr_num);
		}
	}

	// Delete remaining files with tmp CNFs:
	system_str = "rm -f tmp_*.cnf";
	utils::exec(system_str);

	print_solve_stats(processed, vary_vars_truth_table.size(), unsat_num, sat_num, interr_num);
	cout << "Finished" << endl;
	cout << "Elapsed " << elapsedWallTime() << " sec" << endl;
}
