/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef cnf_h
#define cnf_h

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

struct CNF {
	long long int var_num;
	long long int clause_num;
	vector<string> clauses;
	CNF() : var_num(0), clause_num(0), clauses() {}
	CNF(string cnf_name) : var_num(0), clause_num(0), clauses() {
		read(cnf_name);
	}
	void read(string cnf_name) {
		ifstream cnf_file(cnf_name, ios_base::in);
		string str;
		while (getline(cnf_file, str)) {
			if (str.size() == 0 or str[0] == 'p' or str[0] == 'c')
				continue;
			clauses.push_back(str);
			stringstream sstream;
			sstream << str;
			long long int ival;
			while (sstream >> ival)	var_num = max(llabs(ival), var_num);
		}
		cnf_file.close();
        clause_num = clauses.size();
		assert(clause_num > 0);
	}
	void print() {
		cout << "var_num : " << var_num << endl;
		cout << "clause_num : " << clause_num << endl;
	};
};

#endif
