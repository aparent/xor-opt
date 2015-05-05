#include "model.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv) {
    ifstream input(argv[1]);
    Model model(input);
    cout << model.display_str() << endl;
    model.getXorGroups();
    cout << model.display_str() << endl;
    return 0;
}
