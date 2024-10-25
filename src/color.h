#pragma once

#include <cmath>
#include <iostream>

#include "build_target.h"

using std::ostream;

class colorLAB;
class colorRGB;

class colorHSV {
 public:
  colorHSV();
  colorHSV(double hue, double sat, double val);
  colorHSV(const colorHSV& col);

  void setHSV(double hue, double sat, double val);
  colorRGB getRGB() const;

  void invert();

  void operator=(const colorHSV& col);
  bool operator==(const colorHSV& col);

  friend ostream& operator<<(ostream& out, const colorHSV& color);

  double h;
  double s;
  double v;
};

class colorRGB {
 public:
  colorRGB();
  colorRGB(double red, double green, double blue);
  colorRGB(const Color& col);
  colorRGB(const colorLAB& col);

  colorHSV getHSV() const;
  colorLAB getLAB() const;

  bool operator==(const colorRGB& col);

  void setRGB(double red, double green, double blue);
  void setRGB(const colorHSV& hsv);

  void invert();

  friend ostream& operator<<(ostream& out, const colorRGB& color);

  double r;
  double g;
  double b;
};

class colorLAB {
 public:
  colorLAB() : l(0), a(0), b(0) {}
  colorLAB(const colorRGB& color);
  colorLAB(float l, float a, float b) {
    this->l = l;
    this->a = a;
    this->b = b;
  }
  colorLAB(const colorLAB& other) {
    l = other.l;
    a = other.a;
    b = other.b;
  }

  void operator=(const colorLAB& col);
  bool operator==(const colorLAB& col);
  bool operator!=(const colorLAB& col);

  friend ostream& operator<<(ostream& out, const colorLAB& color);

  double l;
  double a;
  double b;
};
