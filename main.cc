#include <iostream>
#include <fstream>
#include <string>

#include "window.h"
#include "note.h"

using std::cerr;
using std::endl;
using std::string;
using std::ifstream;

#define VERSION "0.01a"

int main(int argc, char* argv[]) {

  if (argc !=2) {
    cerr << "Invalid Usage: specify a input file!" << endl;
    exit(1);
  }
  
  string filename = argv[1];

  ifstream filecheck;
  filecheck.open(filename);
  if (!filecheck) {
    cerr << "Invalid File: " << filename << "!" << endl;
    exit(1);
  }
  filecheck.close();

  window main = window(static_cast<string>("mviewer ") +static_cast<string>(VERSION));


  mfile input;
  
  input.load(filename);

  main.clearBuffer();

  return 0;
}
