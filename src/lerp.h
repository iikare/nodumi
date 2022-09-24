#pragma once

#include <raylib.h>
#include "log.h"
#include "enum.h"

double scaleInterpolationRatio(double ratio, int type = INT_LINEAR);


Color colorLERP(Color a, Color b, double ratio, int type = INT_LINEAR);
float floatLERP(float a, float b, double ratio, int type = INT_LINEAR);
