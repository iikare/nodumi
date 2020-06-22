#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "note.h"
#include "color.h"

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::to_string;
using std::transform;
using std::ofstream;
using std::ios;

void saveFile(string path, const vector<colorRGB>& colorVecA, const vector<colorRGB>& colorVecB,
              bool colorByPart, bool drawLine, bool songTime, bool invertColor) {
  // force correct file extension
  transform(path.begin(), path.end(), path.begin(), ::tolower);
  if (path.substr(path.size() - 4) != ".mki") {
    path.append(".mki");
  }
 
  // ensure file is valid
  if (path.size() == 4) {
    cerr << "warn: saving file with no name" << endl;
  }

  // start constructing the file
  char separator = '|';
  string file;
  file += to_string(colorByPart);
  file += to_string(drawLine);
  file += to_string(songTime);
  file += to_string(invertColor);


  ofstream output;
  output.open(path, ios::out | ios::binary);
  if(!output) {
    cerr<< "warn: unable to save file at path " << path <<endl;
    return;
  }

  Uint32 setting = 0x00000000;
  setting += true;//colorByPart;
  setting <<=2;
  setting += true;//drawLine;
  setting <<=2;
  setting += true;//songTime;
  setting <<=2;
  setting += true;//invertColor;
  setting <<=2;

  output.write(reinterpret_cast<char*>(&setting), sizeof(setting));

  // beginning of colorA
  output.write(&separator, sizeof(separator));
  for (int i = 0; i < static_cast<int>(colorVecA.size()); i++) {
    Uint32 color = 0x00000000;
    color += static_cast<Uint8>(colorVecA[i].r);
    color <<= 8;
    color += static_cast<Uint8>(colorVecA[i].g);
    color <<= 8;
    color += static_cast<Uint8>(colorVecA[i].b);
    color <<=8;
    output.write(reinterpret_cast<char*>(&color), sizeof(color));
  }
  // beginning of colorB
  output.write(&separator, sizeof(separator));
  for (int i = 0; i < static_cast<int>(colorVecB.size()); i++) {
    Uint32 color = 0x00000000;
    color += static_cast<Uint8>(colorVecB[i].r);
    color <<= 8;
    color += static_cast<Uint8>(colorVecB[i].g);
    color <<= 8;
    color += static_cast<Uint8>(colorVecB[i].b);
    color <<=8;
    color += 0xFF;
    output.write(reinterpret_cast<char*>(&color), sizeof(color));
  }
  // end of color section
  output.write(&separator, sizeof(separator));

  cerr << file << endl;




  
  cerr << path << endl;
  output.close(); 
}


