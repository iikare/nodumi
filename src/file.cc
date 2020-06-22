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
  
  // file format begins with first three bytes '|' (0x01111100)
  output.write(&separator, sizeof(separator));
  output.write(&separator, sizeof(separator));
  output.write(&separator, sizeof(separator));
  
  // fourth byte encodes settings data
  // top four bits currently unused
  uint8_t setting = 0x00000000;
  setting += colorByPart;
  setting <<=1;
  setting += drawLine;
  setting <<=1;
  setting += songTime;
  setting <<=1;
  setting += invertColor;

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

  // midi data would go here but the data structure is broken

  // end save file
  output.close(); 
}


