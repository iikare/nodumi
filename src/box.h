#pragma once

#include <iostream>

using std::ostream;

struct rect {
  rect(int a, int b, int c, int d) { x = a; y = b; width = c; height = d; }
  int x;
  int y;
  int width;
  int height;

  friend ostream& operator << (ostream& out, const rect& r) {
    out << "{ " << r.x << ", " << r.y << ", " << r.width << ", " << r.height << "} (xywh)";
    return out;
  }
};

struct point {
  point(int a, int b) { x = a; y = b; }
  int x;
  int y;
  
  friend ostream& operator << (ostream& out, const point& p) {
    out << "{ " << p.x << ", " << p.y << "}";
    return out;
  }
};
