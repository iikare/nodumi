#pragma once 

#include <vector>
#include "log.h"
#include "color.h"

struct lchIntermediary {
  float l;
  float c;
  float h;
};

class kMeansPoint {
  public:

  kMeansPoint() : data({0,0,0}), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(colorRGB color) : data(color.getLAB()), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(colorLAB color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}
  kMeansPoint(Color color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}
  
  colorLAB data;
  int cluster = -1;
  double cDist;

  double distance(kMeansPoint& point);
};
