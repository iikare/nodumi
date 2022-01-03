#pragma once

#include <cmath>
#include <iostream>
#include <raylib.h>

using std::ostream;

class colorLAB;

class colorHSV {
  public:
    colorHSV();
    colorHSV(double hue, double sat, double val);
    colorHSV(const colorHSV& col);

    void setHSV(double hue, double sat, double val);

    void invert();


    void operator = (const colorHSV& col);
    bool operator == (const colorHSV& col);

    friend ostream& operator << (ostream& out, colorHSV color);
    
    double h;
    double s;
    double v;
};

class colorRGB {
  public:
    colorRGB();
    colorRGB(double red, double green, double blue);
    colorRGB(Color col);

    colorHSV getHSV();
    colorLAB getLAB();

    bool operator == (const colorRGB& col);

    void setRGB(double red, double green, double blue);
    void setRGB(colorHSV hsv);
    
    void invert();

    friend ostream& operator << (ostream& out, colorRGB color);

    double r;
    double g;
    double b;
};

class colorLAB {
  public:

    colorLAB() : l(0), a(0), b(0) {}
    colorLAB(colorRGB color);



    double l;
    double a;
    double b;
};
