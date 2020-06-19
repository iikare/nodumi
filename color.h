#ifndef COLOR_H
#define COLOR_H

class colorHSV {
  public:
    colorHSV();
    colorHSV(unsigned char hue, unsigned char sat, unsigned char val);
    colorHSV(colorHSV& col);

    void setHSV(unsigned char hue, unsigned char sat, unsigned char val);

    unsigned char h;
    unsigned char s;
    unsigned char v;
};

class colorRGB {
  public:
    colorRGB();
    colorRGB(unsigned char red, unsigned char green, unsigned char blue);

    colorHSV getHSV();

    void setRGB(unsigned char red, unsigned char green, unsigned char blue);
    void setRGB(colorHSV hsv);

    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#endif
