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

class Model {
public:
    Model(std::ifstream &); // Create model from blif input
    std::string display_str();
    std::string getXorGroups();
    enum Gate_Cell { And, Negate_And, Dont_Care };
    class Gate {
    public:
        Gate(std::vector<std::string> _inputs, std::string _output,
             std::vector<std::vector<Gate_Cell>> _exprs)
            : exprs(_exprs), inputs(_inputs), output(_output)  {}
        std::string display_str();
        std::string display_str_expr(std::vector<Gate_Cell> expr);
        std::vector<std::vector<int>> xorGroups();
        std::vector<std::vector<Gate_Cell>> exprs;

    private:
        bool areExclusive(const std::vector<Gate_Cell> &,
                          const std::vector<Gate_Cell> &);
        std::vector<std::string> inputs;
        std::string output;
    };

private:
    std::string name;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<Gate> gates;
    std::vector<std::string> getVars(std::sregex_iterator, std::sregex_iterator);
};

Model::Model(std::ifstream &blif_inp) {
    std::regex dotline_regex(R"([.].*)");
    std::regex var_regex("(\\S+)");
    std::regex wrapped_line_regex(R"(.*\\$)");
    std::string line;
    while (std::getline(blif_inp, line)) {
        // unwrap the \ delimited lines
        while (std::regex_match(line, wrapped_line_regex)) {
            auto temp = line;
            std::getline(blif_inp, line);
            line = temp.substr(0, temp.length() - 2) + line; // -2 to remove the '\'
        }
        if (std::regex_match(line, dotline_regex)) {
            auto vars_begin =
                std::sregex_iterator(line.begin(), line.end(), var_regex);
            auto vars_end = std::sregex_iterator();
            if ((*vars_begin).str() == ".model") {
                name = (*(++vars_begin)).str();
            } else if ((*vars_begin).str() == ".inputs") {
                inputs = getVars(++vars_begin, vars_end);
            } else if ((*vars_begin).str() == ".outputs") {
                outputs = getVars(++vars_begin, vars_end);
            } else if ((*vars_begin).str() == ".names") {
                auto inputs = getVars(++vars_begin, vars_end);
                // the last element of the var list is the output
                auto output = inputs.back();
                inputs.pop_back();
                std::vector<std::vector<Gate_Cell>> gateTable;
                while (blif_inp.peek() != '.') {
                    std::getline(blif_inp, line);
                    std::vector<Gate_Cell> gate_line;
                    for (auto x : line) {
                        if (x == ' ')
                            break;
                        if (x == '-')
                            gate_line.push_back(Dont_Care);
                        if (x == '0')
                            gate_line.push_back(Negate_And);
                        if (x == '1')
                            gate_line.push_back(And);
                    }
                    gateTable.push_back(gate_line);
                }
                gates.push_back(Gate(inputs, output, gateTable));
            }
        } else {
            std::cout << "Error on line: " << line << std::endl;
        }
    }
}

std::vector<std::string> Model::getVars(std::sregex_iterator begin,
                                        std::sregex_iterator end) {
    std::vector<std::string> vars;
    for (auto i = begin; i != end; ++i) {
        vars.push_back(i->str());
    }
    return vars;
}

std::string Model::display_str() {
    std::string output = "";
    output += ".model " + name + "\n";
    output += ".inputs";
    for (auto &x : inputs) {
        output += " " + x;
    }
    output += "\n.outputs";
    for (auto &x : outputs) {
        output += " " + x;
    }
    for (auto &x : gates) {
        output += x.display_str();
    }
    return output;
}

std::string Model::getXorGroups() {
    for (auto &i : gates) {
        std::cout << std::endl << "Clique set:" << std::endl;
        auto cliques = i.xorGroups();
        //Use results to reorder exprs
        std::vector<std::vector<Gate_Cell>> grouped_exprs;
        for (auto &j : cliques) {
            std::cout << "Clique on: ";
            for (auto &k : j) {
                std::cout << k <<" ";
                grouped_exprs.push_back(i.exprs[k]);
            }
            std::cout << std::endl;
        }
        i.exprs = grouped_exprs;
    }
    return "";
}

std::vector<std::vector<int>> Model::Gate::xorGroups() {
    assert(exprs.size() >= 1);
    igraph_t graph;
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&graph, exprs.size(), IGRAPH_UNDIRECTED);
    //label vertices
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        SETVAS(&graph,"name",i,std::to_string(i).c_str());
    }
    //Add Edges
    for (unsigned int i = 0; i < exprs.size(); i++) {
        for (unsigned int j = i + 1; j < exprs.size(); j++) {
            if (areExclusive(exprs[i], exprs[j])) {
                igraph_add_edge(&graph, i, j);
            }
        }
    }
    std::vector<std::vector<int>> result;
    //Put clique groups into result
    while(igraph_vcount(&graph) > 0) {
        igraph_vector_t res;
        igraph_largest_single_clique(&graph, &res);
        std::vector<int> clique;
        for (int j = 0; j < igraph_vector_size(&res); j++) {
            auto vertex_number = VAS(&graph,"name",VECTOR(res)[j]);
            clique.push_back(std::stoi(vertex_number));
        }
        result.push_back(clique);
        struct igraph_vs_t verts;
        verts.type = IGRAPH_VS_VECTOR;
        verts.data.vecptr = &res;
        igraph_delete_vertices(&graph,verts);
        igraph_vector_destroy(&res);
    }
    igraph_destroy(&graph);
    return result;
}

bool Model::Gate::areExclusive(const std::vector<Gate_Cell> &a,
                               const std::vector<Gate_Cell> &b) {
    assert(a.size() == b.size());
    for (unsigned int i = 0; i < a.size(); i++) {
        if (a[i] != b[i] && a[i] != Dont_Care && b[i] != Dont_Care) {
            return true;
        }
    }
    return false;
}

std::string Model::Gate::display_str_expr(std::vector<Gate_Cell> expr) {
    std::string ret = "";
    for (auto &x : expr) {
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
    return ret;
}

std::string Model::Gate::display_str() {
    std::string ret = "";
    ret += "\n.names";
    for (auto &x : inputs) {
        ret += " " + x;
    }
    ret += " " + output;
    for (auto &line : exprs) {
        ret += "\n";
        ret += display_str_expr(line);
    }
    return ret;
}

#endif
