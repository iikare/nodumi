#include <functional>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include "log.h"
#include "color.h"
#include "colorgen.h"

using std::vector;
using std::swap;
using std::cerr;
using std::endl;
using std::bind;
using std::pair;
using std::min;

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {
  vector<pair<int, double>> tmp;
  getColorScheme(n, colorVecA, colorVecB, tmp);
}

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight) {
  
  // check zero n
  if (!n) {
    log3(LL_WARN, "call to getColorScheme with n", n);
  }

  // generate color scheme
  colorHSV off (0, 0.7, 115);
  colorHSV on (0, 0.7, 199);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0/n;

    colorVecA.resize(n);
    colorVecB.resize(n);
  colorVecA.clear();
  colorVecB.clear();

  
  for (int i = 0; i < n; i++) {
    off.h = fmod((off.h + increment),360.0);
    on.h = off.h;

    offRGB.setRGB(off);
    onRGB.setRGB(on);
    colorVecA.push_back(offRGB);
    colorVecB.push_back(onRGB);
  }

  if (weight.size() != 0) {
    for (unsigned int i = 0; i < weight.size(); i++) {
      swap(colorVecA[i], colorVecA[weight[i].first]);
      swap(colorVecB[i], colorVecB[weight[i].first]);
    }
  } 
}

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off){
  invertColor(bg);
  invertColor(line);
  for (unsigned int i = 0; i < on->size(); i++) {
    invertColor(on->at(i));
    invertColor(off->at(i));
  }
}

void invertColor(colorRGB& col) {
  col.r = 255 - col.r;
  col.g = 255 - col.g;
  col.b = 255 - col.b;
}
