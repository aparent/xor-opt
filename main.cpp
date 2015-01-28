#include "model.h"
#include <iostream>
#include <fstream>

using namespace std;

int main() {
  ifstream input("blif-benchmarks/blif_set2/too_large.blif");
  Model model(input);
  cout << model.display_str() << endl;
  model.getXorGroups();
  return 0;
}
