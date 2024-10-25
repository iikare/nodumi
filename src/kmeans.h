#pragma once

#include <vector>

#include "color.h"
#include "log.h"

struct lchIntermediary {
  float l;
  float c;
  float h;
};

class kMeansPoint {
 public:
  kMeansPoint() : data(), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(const colorRGB& color) : data(color.getLAB()), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(const colorLAB& color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(float l, float a, float b) : data(l, a, b), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(const Color& color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}

  colorLAB data;
  int cluster = -1;
  double cDist;

  double distance(const kMeansPoint& point);
};
