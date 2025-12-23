/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef utils_h
#define utils_h

#include <string>
#include <vector>
#include <cassert>

using namespace std;

namespace utils {

    // Execute a system command:
    inline string exec(const string cmd_str) {
        assert(cmd_str != "");
        char* cmd = new char[cmd_str.size() + 1];
        for (unsigned i = 0; i < cmd_str.size(); i++)
            cmd[i] = cmd_str[i];
        cmd[cmd_str.size()] = '\0';
        FILE* pipe = popen(cmd, "r");
        delete[] cmd;
        if (!pipe) return "ERROR";
        char buffer[128];
        string result = "";
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        return result;
    }

    // Generate a truth table with 2^n rows given an integer n:
    inline vector<vector<bool>> generateTruthTable(const unsigned n) {
        assert(n > 0);
        vector<vector<bool>> truth_table;
        unsigned num_rows = 1 << n;
        for (unsigned i = 0; i < num_rows; ++i) {
            vector<bool> row;
            for (int j = n-1; j >= 0; --j) {
                int bit = (i >> j) & 1;
                assert(bit == 0 or bit == 1);
                row.push_back(bit == 1);
            }
            truth_table.push_back(row);
        }
        assert(not truth_table.empty());
        return truth_table;
    }

}

#endif
