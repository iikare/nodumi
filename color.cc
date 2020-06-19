#include <algorithm>
#include <cmath>
#include "color.h"

using std::min;
using std::max;

colorRGB::colorRGB() : r(0), g(0), b(0) {}

colorRGB::colorRGB(double red, double green, double blue) : r(red), g(green), b(blue) {}

void colorRGB::setRGB(double red, double green, double blue) {
  r = red;
  g = green;
  b = blue;
}

void colorRGB::setRGB (colorHSV hsv) {
  // implementation of HSV->RGB algorithm
  
  double chroma = hsv.s * hsv.v;
  double m = hsv.v - chroma;
  double x = chroma * (1 - abs(fmod((hsv.h / 60), 2) - 1));

  if (hsv.h > 0 && hsv.h <= 60) {
    setRGB(chroma + m, x + m, m);
  }
  else if (hsv.h > 60 && hsv.h <= 120) {
    setRGB(x + m, chroma + m, m);
  }
  else if (hsv.h > 120 && hsv.h <= 180) {
    setRGB(m, chroma + m, x + m);
  }
  else if (hsv.h > 180 && hsv.h <= 240) {
    setRGB(m, x + m, chroma + m);
  }
  else if (hsv.h > 240 && hsv.h <= 300) {
    setRGB(x + m, m, chroma + m);
  }
  else if (hsv.h > 300 && hsv.h <= 360) {
    setRGB(chroma + m, m, x + m);
  }
  else {
    setRGB(m, m, m);
  }
}

colorHSV colorRGB::getHSV () {
  // implementation of RGB->HSV algorithm
  
  double value = max({r, g, b});
  double minv = min({r, g, b});
  double chroma =  value - minv;
  double hue = 0;
  double saturation = chroma / value;

  // nonzero hue if nonzero chroma
  if (chroma != 0) {
    if (r == value) {
      // red is max
      hue = fmod(((g - b) / chroma), 6 ) * 60;
    }
    else if (g == value) {
      // green is max
      hue = (((b - r) / chroma) + 2) * 60;
    }
    else if (b == value) {
      // blue is min
      hue = (((r - g) / chroma) + 4) * 60;
    }
  }
  
  colorHSV col (hue, saturation, value);
  return col;
}


colorHSV::colorHSV() : h(0), s(0), v(0) {}

colorHSV::colorHSV(double hue, double sat, double val) : h(hue), s(sat), v(val) {}

colorHSV::colorHSV(const colorHSV& col) {
  h = col.h;
  s = col.s;
  v = col.v;
}

void colorHSV::setHSV(double hue, double sat, double val) {
  h = hue;
  s = sat;
  v = val;
}