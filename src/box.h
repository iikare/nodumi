#pragma once

struct rect {
  rect(int a, int b, int c, int d) { x = a; y = b; width = c; height = d; }
  int x;
  int y;
  int width;
  int height;
};

struct point {
  int x;
  int y;
};
