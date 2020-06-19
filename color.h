#ifndef COLOR_H
#define COLOR_H

class colorHSV {
  public:
    colorHSV();
    colorHSV(double hue, double sat, double val);
    colorHSV(const colorHSV& col);

    void setHSV(double hue, double sat, double val);

    double h;
    double s;
    double v;
};

class colorRGB {
  public:
    colorRGB();
    colorRGB(double red, double green, double blue);

    colorHSV getHSV();

    void setRGB(double red, double green, double blue);
    void setRGB(colorHSV hsv);

    double r;
    double g;
    double b;
};

#endif
