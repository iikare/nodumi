#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>
#include "../dpd/picoPNG/picopng.h"
#include "color.h"
#include "colorgen.h"
#include "misc.h"
#include "box.h"

using std::vector;
using std::string;

class BGImage {
  public:
    BGImage();
    ~BGImage();
    
    void loadPNG(string path);
    void scale(double ratio);
    vector<pixel> getKMeansSample();
    void flip(bool dir);
    void clear();
    void resetPosition() { x = 0; y = 0; }
    void resetScale() { scale(1/scaleRatio); }
    void centerImage(int cX, int cY); 

    colorRGB getPixelRGB(int x, int y);
    int getX() { return x; }
    int getY() { return y; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    rect getBox() { rect result(x, y, width, height); return result; };

    void setX(int nX) { x = nX; }
    void setY(int nY) { y = nY; }
    void setXY(point XY) { x = xOrig + XY.x - xOff; y =  yOrig + XY.y - yOff; }
    void setXYOffset(point XYOff) { xOrig = x; yOrig = y; xOff = XYOff.x; yOff = XYOff.y; };

    void clearXYOffset() { xOff = 0; yOff = 0; xOrig = 0; yOrig = 0; };
  private:
    vector<unsigned char> buffer;
    vector<unsigned char> image;
    vector<unsigned char> oImage;
    vector<pixel> kMeansData;
    int x;
    int y;
    unsigned long width;
    unsigned long height;
    int xOff;
    int yOff;
    int xOrig;
    int yOrig;
    double scaleRatio;
    int oWidth;
    int oHeight;
};

#endif
