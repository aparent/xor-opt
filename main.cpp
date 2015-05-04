#include "model.h"
#include <iostream>
#include <fstream>

using namespace std;

int main() {
  ifstream input("test.blif");
  Model model(input);
  cout << model.display_str() << endl;
  model.getXorGroups();
  return 0;
}
