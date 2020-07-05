#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <iostream>
#include "box.h"

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

    void setRGB(double red, double green, double blue);
    void setRGB(colorHSV hsv);
    void increaseValue(int valInc);

    void operator = (const colorHSV& col);
    friend ostream& operator << (ostream& out, colorRGB color);

    double r;
    double g;
    double b;
};

class colorMenu {
  public:
    colorMenu();
    colorMenu(int iX, int iY, colorRGB color);

    int getX() { return x; }
    int getY() { return y; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getCenterX() { return x + cX; }
    int getCenterY() { return y + cY; }
    int getInner() { return innerRadius; }
    int getOuter() { return outerRadius; }
    int getSquareX() { return x + cX - offset + 2; }
    int getSquareY() { return y + cY - offset + 2; }
    int getSquareSize() { return 2 * offset - 4; }
    int getPointX() { return pX; }
    int getPointY() { return pY; }
    int getPointAngle() { return pAngle; }
    int getHue() { return fmod(colhsv.h + 360, 360); }
    int getSat() { return colhsv.s; }
    int getVal() { return colhsv.v; }
    int getSPointX() { return sX; }
    int getSPointY() { return sY; }
    int getCPointX() { return ciX; }
    int getCPointY() { return ciY; }
    point getCenterXY() { centerXY.x = getCenterX(); centerXY.y = getCenterY(); return centerXY; }
    
    colorRGB getColor() { return col; }
    colorHSV getColorHSV() { return colhsv; }

    void setXY(int nX, int nY) { x = nX; y = nY; }
    void setColor(colorRGB color);
    void setPointX(int nX) { pX = nX; }
    void setPointY(int nY) { pX = nY; }
    void setSPointXY(point XY);
    void setCPointXY(point XY);
    void setPointAngle(int nAngle) { pAngle = nAngle; }

    void findAngleFromColor();
    void findHSVFromSquare(bool findHue);
    rect getBoundingBox();
    rect getBoundingBoxSquare();
    rect getBoundingBoxCircle();

    bool render;
    bool squareClick;
    bool circleClick;
    bool clickBG;

  private:
    int x;
    int y;
    int width;
    int height;
    int cX;
    int cY;
    int innerRadius;
    int outerRadius;
    double offset;
    double pAngle;
    int pX;
    int pY;
    int sX;
    int sY;
    int ciX;
    int ciY;
    colorRGB col;
    colorHSV colhsv;
    rect area;
    rect areaSquare;
    rect areaCircle;
    point centerXY;
};


#endif
