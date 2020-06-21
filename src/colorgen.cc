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
  colorHSV off (0, 0.7, 115);
  colorHSV on (0, 0.7, 199);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0/trackCount;

  colorVecA.clear();
  colorVecB.clear();
  
  for (int i = 0; i < trackCount; i++) {
    off.h = fmod((off.h + increment),360.0);
    on.h = off.h;
    cerr << off.h << " vs. " << on.h << endl; 

    offRGB.setRGB(off);
    onRGB.setRGB(on);
    colorVecA.push_back(offRGB);
    colorVecB.push_back(onRGB);
  }

 return; 
}
