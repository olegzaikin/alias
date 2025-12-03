// Created on: 13 Feb 2018
// Author: Oleg Zaikin
// E-mail: zaikin.icc@gmail.com
//
// Given a CNF and a start decomposition set, find its runtime-wise better
// subset.
// 
// Example:
//    ./alias ./problem.cnf ./dset
//=============================================================================

#include <iostream>
#include "igbfs.h"
#include <cassert>

void writeUsage();
void printVersion();

string version = "1.0.2";

int main(int argc, char *argv[])
{
	vector<string> str_argv;
	for (int i=0; i < argc; ++i) str_argv.push_back(argv[i]);
	assert(str_argv.size() == argc);

	if ( (argc == 2) and (str_argv[1] == "-v") ) {
		printVersion();
		exit(EXIT_SUCCESS);
	}
	else if ( ( (argc == 2) and ( (str_argv[1] == "--help") or (str_argv[1] == "-h") ) ) or 
	          (argc < 4) ) {
		writeUsage();
		exit(EXIT_SUCCESS);
	}
		
	igbfs igbfs_obj;
	igbfs_obj.parseParams(str_argv);
	igbfs_obj.init();
	igbfs_obj.findBackdoor();
	// Don't solve a SAT instance via the found backdoor for a while:
	//igbfs_obj.solveInstance();
	igbfs_obj.reportResult();
	
	return 0;
}

void printVersion()
{
	cout << "version: " << version << endl;
}

void writeUsage()
{
	cout << "USAGE: ./alias <cnf-file> <decomposition-set-file> <incr-sat-solver> [options] \n";
	cout << "decomposition-set-file              [OBLIGATORY] Name of a PCS (Parameter Configuration Space) file with a start point.\n";
	cout << "incr-sat-solver (default: cadical2) [OBLIGATORY] Name of an incremental SAT solver;";
	cout << "\n";
	cout << "CORE OPTIONS: \n";
	cout << "-opt-alg     = <unsigned> (default: 1+1)    [OPTIONAL]   Optimization algorithm.\n";
	cout << "	0: random search in the whole space" << endl;
	cout << "	1: random search reduce size by one" << endl;
	cout << "	2: simple hill climbing (add/remove)" << endl;
	cout << "	3: steepest ascent hill climbing (add/remove)" << endl;
	cout << "	4: tabu search (add/remove)" << endl;
	cout << "	5: (1+1)-EA" << endl;
	cout << "	6: iterated hill climbing with variables-based jump" << endl;
	cout << "	7: simple hill climbing (add/remove/partial-replace)" << endl;
	cout << "	8: (1+1)-EA with simple hill climbing (add/remove/partial-replace)" << endl;
	cout << "	9: simple hill climbing (add/remove/replace)" << endl;
	cout << "-backdoor    = <string>                     [OPTIONAL]   Name of a file that contains a backdoor (numeration from 1) \n";
	cout << "--solve                                     [OPTIONAL]   Enable solving of a given instance by a found (or a given) backdoor \n";
	cout << "MAIN OPTIONS: \n";
	cout << "-cpu-lim     = <int> (default: 3600)        [OPTIONAL]   CPU wall time limit in seconds \n";
	cout << "-seed        = <int32> (default: 0)         [OPTIONAL]   Random seed \n";
	cout << "-verb        = <int32> [0..2] (default: 1)  [OPTIONAL]   Verbosity \n";
	cout << "--help       Print help message. \n";
}
