#pragma once

#include <vector>
#include "color.h"

using std::vector;
using std::pair;

class pixel;

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off);

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         vector<pair<int, double>>& weight);
void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

vector<colorRGB> findKMeans(vector<pixel>& colorData, int k, int& meanV);
vector<colorRGB> findKMeansMultithreaded(vector<pixel>& colorData, int k, int& meanV);

class pixel {
  public:

  pixel() : data(0,0,0), cluster(-1), cDist(__DBL_MAX__) {}
  pixel(colorRGB color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}
  pixel(Color color) : data(color), cluster(-1), cDist(__DBL_MAX__) {}
  
  colorRGB data;
  int cluster = -1;
  double cDist;

  double distance(pixel point) {
    //sqrt omitted for efficiency
    return pow(point.data.r - data.r, 2) + pow(point.data.g - data.g, 2) + pow(point.data.b - data.b, 2);
  }
  private:

};
