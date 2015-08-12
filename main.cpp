#include "model.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        ifstream input(argv[i]);
        Model model;
        if ( model.parse_model(input) ) {
            model.getXorGroups();
            ofstream file;
            file.open("processed-blif/" + string(argv[i]));
            file << model.display_str();
            file.close();
        }
    }
    return 0;
}
