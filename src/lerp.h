#pragma once

#include "build_target.h"

#include "log.h"
#include "enum.h"
#include "color.h"

using std::is_same;

double scaleInterpolationRatio(double ratio, int type = INT_LINEAR);


template<class T>
T colorLERP(const T& a, const T& b, double ratio, int type) {
  ratio = scaleInterpolationRatio(ratio, type);

  if constexpr (is_same<T, Color>::value) {
    return (Color) {(unsigned char)(a.r*(1-ratio)+b.r*(ratio)),
                    (unsigned char)(a.g*(1-ratio)+b.g*(ratio)),
                    (unsigned char)(a.b*(1-ratio)+b.b*(ratio)),
                    (unsigned char)(a.a*(1-ratio)+b.a*(ratio))}; 
  }
  if constexpr (is_same<T, colorRGB>::value) {
    return (colorRGB){(a.r*(1-ratio)+b.r*(ratio)),
                      (a.g*(1-ratio)+b.g*(ratio)),
                      (a.b*(1-ratio)+b.b*(ratio))};
  }

  logW(LL_WARN, "call to", __func__, "using invalid type", typeid(T).name());
  return T();
}
//Color colorLERP(const Color& a, Color& b, double ratio, int type = INT_LINEAR);
//colorRGB colorLERP(colorRGB& a, colorRGB& b, double ratio, int type = INT_LINEAR);
float floatLERP(float a, float b, double ratio, int type = INT_LINEAR);

template<class T>
T valueLERP(T a, T b, double ratio, int type = INT_LINEAR) {
  ratio = scaleInterpolationRatio(ratio, type);

  return a*ratio + b*(1.0-ratio);
}
