#include <vector>
using namespace std;
#include "input.h"

InputParser::InputParser(const int &argc, char **argv) {
    for (int i = 1; i < argc; ++i)
        this->tokens.push_back(string(argv[i]));
}

const string& InputParser::getCmdOption(const string &option) const {
    vector<string>::const_iterator itr;
    itr = find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
        return *itr;
    }
    static const string empty_string("");
    return empty_string;
}

bool InputParser::cmdOptionExists(const string &option) const {
    return find(this->tokens.begin(), this->tokens.end(), option)
        != this->tokens.end();
}
