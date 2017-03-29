#pragma once

class InputParser{
public:
    InputParser(const int &argc, char **argv);
    const string& getCmdOption(const string &option) const;
    bool cmdOptionExists(const string &option) const;

private:
    vector <string> tokens;
};
