#pragma once 

#include "color.h"

using std::pair;

struct particleInfo {
  double x; 
  double y;
  colorRGB col;

  friend ostream& operator << (ostream& out, const particleInfo& p) {
    out << "{" << p.x << ", " << p.y << ", " << p.col << "}"; 
    return out;
  }
};

struct set_comp {
    bool operator() (const pair<int, const particleInfo>& a, const pair<int, particleInfo>& b) const {
        return a.first < b.first;
    }
};
