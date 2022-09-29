#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include "log.h"
#include "enum.h"
#include "color.h"

double scaleInterpolationRatio(double ratio, int type = INT_LINEAR);


Color colorLERP(Color a, Color b, double ratio, int type = INT_LINEAR);
colorRGB colorLERP(colorRGB a, colorRGB b, double ratio, int type = INT_LINEAR);
float floatLERP(float a, float b, double ratio, int type = INT_LINEAR);
