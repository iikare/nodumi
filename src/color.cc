#include <algorithm>
#include <iostream>
#include <cmath>
#include "color.h"
#include "misc.h"
#include "log.h"

using std::min;
using std::max;
using std::cerr;
using std::endl;
using std::ostream;

colorRGB::colorRGB() : r(0), g(0), b(0) {}

colorRGB::colorRGB(double red, double green, double blue) : r(red), g(green), b(blue) {}

colorRGB::colorRGB(const Color& col) : r(col.r), g(col.g), b(col.b) {}

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

colorLAB colorRGB::getLAB() {
  
  colorLAB result;

  // first, convert the color components to a range in [0,1]
  
  float convR = r / 255.0f;
  float convG = g / 255.0f;
  float convB = b / 255.0f;

  // assuming sRGB -> CIEXYZ conversion
  const auto linearizeGamma = [] (float value) {
    if (value <= 0.04045f) {
      return value/12.92f;
    }
    return (float)pow((value+0.055)/1.055,2.4);
  };
  
  //logQ(convR, convG, convB);
  convR = linearizeGamma(convR);
  convG = linearizeGamma(convG);
  convB = linearizeGamma(convB);
  //logQ(convR, convG, convB);

  // multiply by defined 3x3 sRGB->CIEXYZ matrix
  // 
  // 
  // 0.4124 0.3576 0.1805  | R
  // 0.2126 0.7152 0.0722  | G
  // 0.0193 0.1192 0.9505  | B

  // sRGB D65
  constexpr float tMat[3][3] = {{0.4124,0.3576,0.1805},
                                {0.2126,0.7152,0.0722},
                                {0.0193,0.1192,0.9505}};

  // sRGB D50
  //constexpr float tMat[3][3] = {{0.4361,0.3851,0.1431},
                                //{0.2225,0.7169,0.0606},
                                //{0.0139,0.0971,0.7142}};
 
  // sRGB D65 
  constexpr float cieX_n = 95.0489f / 100.0f;
  constexpr float cieY_n = 100.0f / 100.0f;
  constexpr float cieZ_n = 108.8840f / 100.0f;
  
  // sRGB D50 
  //constexpr float cieX_n = 96.42f / 100.0f;
  //constexpr float cieY_n = 100.0f / 100.0f;
  //constexpr float cieZ_n = 82.51f / 100.0f;

  constexpr float delta = 6.0f/29.0f;

  float cieX = tMat[0][0]*convR + tMat[0][1]*convG + tMat[0][2]*convB;
  float cieY = tMat[1][0]*convR + tMat[1][1]*convG + tMat[1][2]*convB;
  float cieZ = tMat[2][0]*convR + tMat[2][1]*convG + tMat[2][2]*convB; 

  // compute t value for each 
  float cieXX_n = cieX/cieX_n;
  float cieYY_n = cieY/cieY_n;
  float cieZZ_n = cieZ/cieZ_n;

  // t transformation function
  const auto cieTransform = [&] (float t) {
    if (t > pow(delta, 3)) {
      return pow(t, 1.0f/3.0f);
    }
    return t/(3*pow(delta,2)) + 4.0f/29.0f;
  };
  
  // t transform all values
  float fXX_n = cieTransform(cieXX_n);
  float fYY_n = cieTransform(cieYY_n);
  float fZZ_n = cieTransform(cieZZ_n);

  // compute final LAB values (l: [0,100] a:[-128,128] b:[-128,128])

  result.l = 116*fYY_n - 16;
  result.a = 500*(fXX_n-fYY_n);
  result.b = 200*(fYY_n-fZZ_n);

  #ifndef NO_DEBUG
  if (result.l < 0 || result.l > 100) { logQ("l out of bounds:", result.l); }
  if (fabs(result.a) > 128) { logQ("a out of bounds:", result.a); }
  if (fabs(result.b) > 128) { logQ("b out of bounds:", result.b); }

  #endif  

  return result;
}

colorRGB::colorRGB(const colorLAB& col) {


  // convert from CIELAB to CIEXYZ
  constexpr float delta = 6.0f/29.0f;
  
  // inverse t transformation function
  const auto cieTransform = [&] (float t) {
    if (t > delta) {
      return pow(t, 3.0f);
    }
    return 3*pow(delta,2)*(t - 4.0f/29.0f);
  };
  
  // sRGB D65
  constexpr float tMat[3][3] = {{ 3.2406,-1.5372,-0.4986},
                                {-0.9689, 1.8758, 0.0415},
                                { 0.0557,-0.2040, 1.0570}};
 
  // sRGB D65 
  constexpr float cieX_n = 95.0489f / 100.0f;
  constexpr float cieY_n = 100.0f / 100.0f;
  constexpr float cieZ_n = 108.8840f / 100.0f;

  const float lPrime = (col.l + 16)/116;
  const float aPrime = col.a/500;
  const float bPrime = col.b/200;


  const float cieX = cieX_n * cieTransform(lPrime + aPrime);
  const float cieY = cieY_n * cieTransform(lPrime);
  const float cieZ = cieZ_n * cieTransform(lPrime - bPrime);

  // now in CIEXYZ, convert to sRGB
  
  const float rLin = tMat[0][0]*cieX + tMat[0][1]*cieY + tMat[0][2]*cieZ;
  const float gLin = tMat[1][0]*cieX + tMat[1][1]*cieY + tMat[1][2]*cieZ;
  const float bLin = tMat[2][0]*cieX + tMat[2][1]*cieY + tMat[2][2]*cieZ; 



  const auto delinearizeGamma = [] (float value) {
    if (value <= 0.0031308f) {
      return value*12.92f;
    }
    return 1.055f*(float)pow(value, 1.0f/2.4f) - 0.055f;
  };

  //const float r = delinearizeGamma(rLin);
  //const float g = delinearizeGamma(gLin);
  //const float b = delinearizeGamma(bLin);
  
  // convert to [0,255] range
  r = 255*delinearizeGamma(rLin);
  g = 255*delinearizeGamma(gLin);
  b = 255*delinearizeGamma(bLin);



}



colorLAB::colorLAB(colorRGB color) {

  colorLAB col = color.getLAB();
 

  l = col.l;
  a = col.a;
  b = col.b;
}

void colorLAB::operator = (const colorLAB& col) {
  l = col.l;
  a = col.a;
  b = col.b;
}

bool colorLAB::operator == (const colorLAB& col) {
  return l == col.l && a == col.a && b == col.b; 
}
bool colorLAB::operator != (const colorLAB& col) {
  return !(l == col.l && a == col.a && b == col.b); 
}

ostream& operator << (ostream& out, colorLAB color) {
  out << "{" << color.l << ", " << color.a << ", " << color.b << "} (LAB)";
  return out;
}
