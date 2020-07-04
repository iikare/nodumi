#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>
#include <fstream>
#include "note.h"

using std::string;
using std::vector;
using std::ifstream;

void saveFile(string path, mfile* file, const vector<colorRGB>& colorVecA, const vector<colorRGB>& colorVecB,
              colorRGB& bg, int displayMode, int colorMode, bool drawLine, bool songTime, bool invertColor);

bool checkMKI(ifstream& file, string path);

bool readFromFile(ifstream& in, char* data, int width);

#endif
