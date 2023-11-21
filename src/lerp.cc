#include "lerp.h"
#include <algorithm>
#include <cmath>

double scaleInterpolationRatio(double ratio, int type) {
    switch (type) {
      case INT_CIRCULAR:
        return sqrt(1 - pow(ratio, 2));
      case INT_ICIRCULAR:
        return 1-sqrt(1 - pow(ratio, 2));
      case INT_ISINE:
        return 1 - sin(ratio * M_PI / 2);
      case INT_SINE:
        return sin(ratio * M_PI / 2);
      case INT_SSTEP:
        if (ratio < 0.5) {
          return 12 * pow(ratio, 2) - 16 * pow(ratio, 3);
        }
        return 1 - (12 * pow(ratio-0.5, 2) - 16 * pow(ratio-0.5, 3));
      case INT_QUADRATIC:
        return pow(ratio, 2);
      case INT_IQUADRATIC:
        return 1-pow(ratio, 2);
      case INT_ILINEAR:
        return 1-ratio;
      default:
        // INT_LINEAR
        return ratio;
  }
}

//Color colorLERP(const Color& a, const Color& b, double ratio, int type) {
  //ratio = scaleInterpolationRatio(ratio, type);
  
  //return (Color) {(unsigned char)(a.r*(1-ratio)+b.r*(ratio)),
                  //(unsigned char)(a.g*(1-ratio)+b.g*(ratio)),
                  //(unsigned char)(a.b*(1-ratio)+b.b*(ratio)),
                  //(unsigned char)(a.a*(1-ratio)+b.a*(ratio))}; 
  
//}

//colorRGB colorLERP(const colorRGB& a, const colorRGB& b, double ratio, int type) {
  //ratio = scaleInterpolationRatio(ratio, type);

  //return colorRGB((a.r*(1-ratio)+b.r*(ratio)),
                  //(a.g*(1-ratio)+b.g*(ratio)),
                  //(a.b*(1-ratio)+b.b*(ratio)));
//}

float floatLERP(float a, float b, double ratio, int type) {
  ratio = scaleInterpolationRatio(ratio, type);

  return a*ratio + b*(1.0-ratio);
}
