#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <regex>
using namespace std; 


int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./preprocessor <filename.s> [-cycles N]\n";
        return 1;
    }
    string filename = argv[1];
    vector<string> final_lines;
    cout << "Pre-processing file: " << filename << endl;
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return 1;
    }
    string line;
    map<string, long long> labels;
    long long line_number = 0;
    long long data_index =  0;
    while (getline(infile, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));
        if (line[0]=='.'){
            size_t colon_pos = line.find(':');
            if (colon_pos != string::npos) {
                string name = line.substr(1, colon_pos - 1);
                labels[name] = data_index;
            string components = line.substr(colon_pos + 1);
            stringstream ss(components);
            string num;
            while (ss >> num) {
                final_lines.push_back(num);
                data_index++;
            }
        }
        }else{
            if(line.find(':') != string::npos){
                size_t colon_pos = line.find(':');
                string label_name = line.substr(0, colon_pos);
                labels[label_name] = line_number;
            }else{
                final_lines.push_back(line);
                line_number++;
            }
        }
    }
    infile.close();
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open file for writing.\n";
        return 1;
    }
    for (string& line : final_lines) {
        for (auto const& [label_name, label_val] : labels){
            string number_str = to_string(label_val);
            line = regex_replace(line, regex("\\b" + label_name + "\\b"), number_str);
        }
        outfile << line << endl;
    }
    outfile.close();
}