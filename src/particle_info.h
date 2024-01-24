#pragma once

#include "color.h"

using std::pair;

struct particleInfo {
  double x;
  double y;
  double w;
  double h;
  colorRGB col;
  colorRGB col_inv;

  friend ostream& operator<<(ostream& out, const particleInfo& p) {
    out << "{" << p.x << ", " << p.y << ", " << p.w << ", " << p.h << ", "
        << p.col << ", " << p.col_inv << "}";
    return out;
  }
};

struct set_comp {
  bool operator()(const pair<int, const particleInfo>& a,
                  const pair<int, particleInfo>& b) const {
    return a.first < b.first;
  }
};
