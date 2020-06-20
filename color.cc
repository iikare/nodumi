#include <algorithm>
#include <iostream>
#include <cmath>
#include "color.h"
#include "misc.h"

using std::min;
using std::max;
using std::cerr;
using std::endl;


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
  double x = chroma * (1 - fabs(fmod((hsv.h / 60), 2) - 1));

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

void colorRGB::increaseValue(int valInc) {
  colorHSV tmp = getHSV();
  tmp.v = min(360.0, tmp.v + valInc);
  setRGB(tmp);
}



colorHSV::colorHSV() : h(0), s(0), v(0) {}

colorHSV::colorHSV(double hue, double sat, double val) : h(hue), s(sat), v(val) {}

void colorHSV::operator = (const colorHSV& col) {
  h = col.h;
  s = col.s;
  v = col.v;
}

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

colorMenu::colorMenu() : render(false), x(0), y(0), width(0), height(0), cX(0), cY(0),
                         innerRadius(0), outerRadius(0), offset(0), pAngle(0), pX(0),
                         pY(0), col({0, 0, 0}), hue(0) {}

colorMenu::colorMenu(int iX, int iY, colorRGB color) : render(false), x(iX), y(iY),
                     width(COLOR_WIDTH), height(COLOR_HEIGHT), cX(iX + COLOR_WIDTH/2), cY(iY + COLOR_HEIGHT/2),
                     col(color) {
  innerRadius = min(COLOR_WIDTH, COLOR_HEIGHT) * 0.4;
  outerRadius = innerRadius + 6;
  offset = ceil(innerRadius / sqrt(2));

  colorHSV h = color.getHSV();
  hue = h.h;
}

void colorMenu::findAngleFromColor() {
  colorHSV tmp = col.getHSV();
  pAngle = tmp.h;
  
  

  pX = getCenterX() + cos(pAngle) * ((innerRadius + outerRadius)/2 - 1);
  pY = getCenterY() + sin(pAngle) * ((innerRadius + outerRadius)/2 - 1);
}

void colorMenu::setColor(colorRGB color) {
  col = color;
  colorHSV h = color.getHSV();
  hue = h.h;
}
