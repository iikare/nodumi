#ifndef COLORGEN_H
#define COLORGEN_H

#include <vector>
#include "color.h"

using std::vector;

class BGImage;

struct pixel {
  colorRGB data;
  int cluster;
  double cDist;

  pixel() : data({0, 0, 0}), cluster(-1), cDist(__DBL_MAX__) {}
  pixel(colorRGB color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}

  double distance(const pixel& point) {
    //sqrt omitted for efficiency
    return pow(point.data.r - data.r, 2) + pow(point.data.g - data.g, 2) + pow(point.data.b - data.b, 2);
  }
};

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorSchemeBG(BGImage* image, int k, int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
vector<colorRGB> findKMeans(vector<pixel>& colorData, int k, int& meanV);

#endif
