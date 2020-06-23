#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

using std::string;
using std::vector;

void saveFile(string path, const note* notes, int noteCount, double timeScale, const vector<colorRGB>& colorVecA, const vector<colorRGB>& colorVecB,
              bool colorByPart, bool drawLine, bool songTime, bool invertColor);
#endif
