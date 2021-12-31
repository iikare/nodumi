#pragma once

#include <cmath>
#include <iostream>

using std::ostream;

class colorHSV {
  public:
    colorHSV();
    colorHSV(double hue, double sat, double val);
    colorHSV(const colorHSV& col);

    void setHSV(double hue, double sat, double val);

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

    colorHSV getHSV();
    
    bool operator == (const colorRGB& col);

    void setRGB(double red, double green, double blue);
    void setRGB(colorHSV hsv);

    friend ostream& operator << (ostream& out, colorRGB color);

    double r;
    double g;
    double b;
};
