#include <iostream>
#include <vector>
#include "color.h"

using std::vector;
using std::cerr;
using std::endl;

void getColorScheme(int trackCount, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {

  // check zero track
  
  if (!trackCount) {
    cerr << "error: call to getColorScheme with zero tracks" << endl;
    exit(1);
  }

  // generate color scheme
  colorHSV off (0, 166, 166);
  colorHSV on (0, 166, 233);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0/trackCount;

  colorVecA.clear();
  colorVecB.clear();
  
  for (int i = 0; i < trackCount; i++) {
    off.h = static_cast<int>(off.h + increment) % 360;
    on.h = static_cast<int>(on.h + increment) % 360;

    offRGB = off;
    onRGB = on;
    colorVecA.push_back(offRGB);
    colorVecB.push_back(onRGB);
  }

 return; 
}
