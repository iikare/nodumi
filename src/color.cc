#include <algorithm>
#include <iostream>
#include <cmath>
#include "color.h"
#include "misc.h"

using std::min;
using std::max;
using std::cerr;
using std::endl;
using std::ostream;

colorRGB::colorRGB() : r(0), g(0), b(0) {}

colorRGB::colorRGB(double red, double green, double blue) : r(red), g(green), b(blue) {}

colorRGB::colorRGB(Color col) : r(col.r), g(col.g), b(col.b) {}

ostream& operator << (ostream& out, colorRGB color) {
  out << "{" << color.r << ", " << color.g << ", " << color.b << "} (RGB)";
  return out;
}

void colorRGB::setRGB(double red, double green, double blue) {
  r = red;
  g = green;
  b = blue;
}

void colorRGB::setRGB (colorHSV hsv) {
  // implementation of HSV->RGB algorithm
  
  *this = HSVtoRGB(hsv);
}

bool colorRGB::operator == (const colorRGB& col) {
  return (int)r == (int)col.r && (int)g == (int)col.g && (int)b == (int)col.b;
}

void colorRGB::invert() {
  r = 255 - r;
  g = 255 - g;
  b = 255 - b;
}

colorHSV colorRGB::getHSV () {
  // implementation of RGB->HSV algorithm
  
  return RGBtoHSV(*this); 
}

colorHSV::colorHSV() : h(0), s(0), v(0) {}

colorHSV::colorHSV(double hue, double sat, double val) : h(hue), s(sat), v(val) {}

void colorHSV::operator = (const colorHSV& col) {
  h = col.h;
  s = col.s;
  v = col.v;
}

bool colorHSV::operator == (const colorHSV& col) {
  if (h == col.h && s == col.s && v == col.v) {
    return true;
  }
  return false;
}

colorHSV::colorHSV(const colorHSV& col) {
  h = col.h;
  s = col.s;
  v = col.v;
}

ostream& operator << (ostream& out, colorHSV color) {
  out << "{" << color.h << ", " << color.s << ", " << color.v << "} (HSV)";
  return out;
}

void colorHSV::setHSV(double hue, double sat, double val) {
  h = hue;
  s = sat;
  v = val;
}

void colorHSV::invert() {
  h = 255 - h;
  s = 255 - s;
  v = 255 - v;
}
