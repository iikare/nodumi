#include <algorithm>
#include "color.h"

using std::min;
using std::max;

colorRGB::colorRGB() : r(0), g(0), b(0) {}

colorRGB::colorRGB(unsigned char red, unsigned char green, unsigned char blue) : r(red), g(green), b(blue) {}

void colorRGB::setRGB(unsigned char red, unsigned char green, unsigned char blue) {
  r = red;
  g = green;
  b = blue;
}

void colorRGB::setRGB (colorHSV hsv) {
  // implementation of HSV->RGB algorithm
}

colorHSV colorRGB::getHSV () {
  // implementation of RGB->HSV algorithm
  
  unsigned char value = max({r, g, b});
  unsigned char minv = min({r, g, b});
  unsigned char chroma =  value - minv;
  unsigned char hue = 0;
  unsigned char saturation = chroma / value;

  // nonzero hue if nonzero chroma
  if (chroma != 0) {
    if (r == value) {
      // red is max
      hue = (((g - b) / chroma) % 6 ) * 60;
    }
    else if (g == value) {
      // green is max
      hue = (((b - r) / chroma) + 2) * 60;
    }
    else if (b == minv) {
      // blue is min
      hue = (((r - g) / chroma) + 4) * 60;
    }
  }
  
  colorHSV col (hue, saturation, value);
  return col;
}


colorHSV::colorHSV() : h(0), s(0), v(0) {}

colorHSV::colorHSV(unsigned char hue, unsigned char sat, unsigned char val) : h(hue), s(sat), v(val) {}

colorHSV::colorHSV(colorHSV& col) {
  h = col.h;
  s = col.s;
  v = col.v;
}

void colorHSV::setHSV(unsigned char hue, unsigned char sat, unsigned char val) {
  h = hue;
  s = sat;
  v = val;
}
