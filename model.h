#ifndef MODEL
#define MODEL

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <igraph/igraph.h>
#include <cassert>

class Model
{
public:
    Model(std::ifstream&); //Create model from blif input
    std::string display_str();
    class Gate
    {
    public:
        enum Gate_Cell { And, Negate_And , Dont_Care };
        Gate(std::vector<std::string> _inputs, std::string _output, std::vector<std::vector<Gate_Cell> > _exps)
            : inputs(_inputs) , output(_output) , exps(_exps) {}
        std::string display_str();
    private:
        bool areExclusive (const std::vector<Gate_Cell>&,const std::vector<Gate_Cell>&);
        std::vector<std::string> inputs;
        std::string output;
        std::vector<std::vector<Gate_Cell> > exps;
    };
private:
    std::string name;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<Gate> gates;
    std::vector<std::string> getVars(std::sregex_iterator,std::sregex_iterator);
};

Model::Model(std::ifstream& blif_inp)
{
    std::regex dotline_regex(R"([.].*)");
    std::regex var_regex("(\\S+)");
    std::regex wrapped_line_regex(R"(.*\\$)");
    std::string line;
    while ( std::getline (blif_inp,line))
    {
        //unwrap the \ delimited lines
        while ( std::regex_match(line,wrapped_line_regex))
        {
            auto temp = line;
            std::getline(blif_inp,line);
            line = temp.substr(0,temp.length()-2) + line; // -2 to remove the '\'
        }
        if (std::regex_match(line,dotline_regex)) {
            auto vars_begin = std::sregex_iterator(line.begin(), line.end(), var_regex);
            auto vars_end = std::sregex_iterator();
            if ( (*vars_begin).str() == ".model") {
                name = (*(++vars_begin)).str();
            }
            else if ((*vars_begin).str() == ".inputs") {
                inputs = getVars(++vars_begin,vars_end);
            }
            else if ((*vars_begin).str() ==  ".outputs") {
                outputs = getVars(++vars_begin,vars_end);
            }
            else if ((*vars_begin).str() == ".names") {
                auto inputs = getVars(++vars_begin,vars_end);
                auto output = inputs.back(); // the last element of the var list is the output
                std::vector<std::vector<Gate::Gate_Cell> > gateTable;
                inputs.pop_back();
                while (blif_inp.peek() != '.') {
                    std::getline (blif_inp,line);
                    std::vector<Gate::Gate_Cell> gate_line;
                    for (auto x : line) {
                        if (x == ' ') break;
                        if (x == '-') gate_line.push_back(Gate::Dont_Care);
                        if (x == '0') gate_line.push_back(Gate::Negate_And);
                        if (x == '1') gate_line.push_back(Gate::And);
                    }
                    gateTable.push_back(gate_line);
                }
                gates.push_back(Gate(inputs,output,gateTable));
            }
        } else {
            //std::cout << line << std::endl;
        }
    }
}

std::vector<std::string> Model::getVars(std::sregex_iterator begin, std::sregex_iterator end) {
    std::vector<std::string> vars;
    for(auto i = begin; i != end; ++i) {
        vars.push_back(i->str());
    }
    return vars;
}

std::string Model::display_str()
{
    std::string output = "";
    output += "name: " + name + "\n";
    output += "inputs:";
    for (auto &x : inputs) {
        output += " " + x;
    }
    output += "\nOutputs:";
    for (auto &x : outputs) {
        output += " " + x;
    }
    output += "\nGates:";
    for (auto &x : gates) {
        output += x.display_str();
    }
    return output;
}




bool Model::Gate::areExclusive  (const std::vector<Gate_Cell> &a,
                               const std::vector<Gate_Cell> &b){
    assert (a.size() == b.size());
    for (int i = 0; i < a.size(); i++){
        if ( a[i] != b[i] && a[i] != Dont_Care &&  b[i] != Dont_Care){
            return true;
        }
    }
    return false;
}

std::string Model::Gate::display_str()
{
    std::string ret = "";
    ret += "\ninputs:";
    for (auto &x : inputs) {
        ret += " " + x;
    }
    ret += "\nOutput: " + output;
    ret += "\nLines:";
    for (auto &line : exps) {
        ret += "\n";
        for (auto &x : line) {
            switch (x) {
            case Dont_Care:
                ret += "-";
                break;
            case And:
                ret += "1";
                break;
            case Negate_And:
                ret += "0";
                break;
            }
        }
    }
    return ret;
}

#endif
