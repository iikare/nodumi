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

void saveFile(string path, const note* notes, int noteCount, double timeScale, const vector<colorRGB>& colorVecA, const vector<colorRGB>& colorVecB,
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
  char noteSeparator = '~';
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

  for (int i = 0; i < noteCount; i++) {
    // each note needs 26 + 1 bytes to encode: broken down as follows
    // 1 byte: note separator (0x01111110)
    // 1 byte: track (unsigned char)
    // 8 bytes: tempo (double)
    // 8 bytes: duration (double)
    // 8 bytes: x (double)
    // 1 bytes: y (unsigned char)
    // note: this limits the amount of tracks to 256
    // note: this limits the Y value to 0 - 255
    
    double dur = notes[i].duration/timeScale; 
    double correctedX = notes[i].x;

    if (notes[i].track > 255 || notes[i].y > 255) {
      cerr << "warn: file attributes exceed limits" << endl;
    }
    
    output.write(&noteSeparator, sizeof(noteSeparator));
    output.write(const_cast<char*>(reinterpret_cast<const char*>(&notes[i].track)), sizeof(uint8_t));
    output.write(const_cast<char*>(reinterpret_cast<const char*>(&notes[i].tempo)), sizeof(double));
    output.write(const_cast<char*>(reinterpret_cast<const char*>(&notes[i].duration)), sizeof(double));
    output.write(const_cast<char*>(reinterpret_cast<const char*>(&notes[i].x)), sizeof(double));
    output.write(const_cast<char*>(reinterpret_cast<const char*>(&notes[i].y)), sizeof(uint8_t));

  }  

  // end save file
  output.close(); 
}
