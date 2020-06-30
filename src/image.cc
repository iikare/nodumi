#include <vector>
#include <string>
#include <iostream>
#include "color.h"
#include "image.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

BGImage::BGImage() : buffer(0), image(0), oImage(0), x(0), y(0), width(0), height(0), xOff(0), yOff(0), xOrig(0), yOrig(0) {} 

BGImage::~BGImage() {}

void BGImage::loadPNG(string path) {
  cerr << "info: loading PNG - " << path << endl;

  // reset variables
  buffer.clear();
  image.clear();
  oImage.clear();
  x = 0; 
  y = 0;
  width = 0;
  height = 0;
  xOff = 0;
  yOff = 0;

  loadFile(buffer, path);

  int error = decodePNG(image, width, height, buffer.empty() ? 0 : &buffer[0], static_cast<unsigned long>(buffer.size()));
  oImage = image;
  if (error != 0) {
    cerr << "warn: png decode error: " << error << endl;
  }
  cerr << "width, height: " << width << ", " << height << endl;
  cerr << "RGBA: " << (int)image[0] << ", " << (int)image[1] << ", " << (int)image[2] << ", " << (int)image[3] << endl;

}

colorRGB BGImage::getPixelRGB(int x, int y, bool useOriginal) {
  // each pixel is 4 bytes
  int offset = y * width  * 4 + x * 4;
  colorRGB col;

  if (useOriginal) {
    col.setRGB(oImage[offset], oImage[offset + 1], oImage[offset + 2]);
  }
  else {
    col.setRGB(image[offset], image[offset + 1], image[offset + 2]);
  }

  return col;
}

void BGImage::setPixelRGB(int x, int y, colorRGB color, int nWidth) {
  int offset;
  if (nWidth == 0) {
    offset = y * width * 4 + x * 4;
  }
  else{
    offset = y * nWidth * 4 + x * 4;
  }
  image[offset] = color.r;
  image[offset + 1] = color.g;
  image[offset + 2] = color.b;
  image[offset + 3] = 255;
}

void BGImage::scale(double ratio) {
  // implementation of bilinear scaling algorithm
  if (ratio < 0 || ratio > 3) {
    cerr << "warn: invalid call to scale() with ratio " << ratio << endl;
  }
  ratio = 0.5;
 cerr << "owo" << endl; 
  /// get rid of previous image
  //image.clear(); 

  double nWidth = width * ratio;
  double nHeight = height * ratio;
  int offset = 0;
  int nRatio = 1.0 / ratio;
  int iX, iY;
  double deltaX, deltaY;

  cerr << "new res: " << nWidth << " x " << nHeight << endl;

  colorRGB a, b, c, d;

  // resize vector to new image
  //image.resize(int(nWidth * nHeight));

  for (int i = 0; i < nHeight; i++) {
    for (int j = 0; j < nWidth; j++) {
      iX = static_cast<int>(j * nRatio);
      iY = static_cast<int>(i * nRatio);
      deltaX = (j * nRatio) - iX;
      deltaY = (i * nRatio) - iY;

      a = getPixelRGB(iX, iY, true);
      b = getPixelRGB(iX + 1, iY, true);
      c = getPixelRGB(iX, iY + 1, true);
      d = getPixelRGB(iX + 1, iY + 1, true);


      double red, green, blue;
     // blue = a.b * (1 - deltaX) * (1 - deltaY) + b.b * deltaX * (1 - deltaY) + c.b * (1 - deltaX) * deltaY + d.b * deltaX * deltaY;
      //green = a.g * (1 - deltaX) * (1 - deltaY) + b.g * deltaX * (1 - deltaY) + c.g * (1 - deltaX) * deltaY + d.g * deltaX * deltaY;
      //red = a.r * (1 - deltaX) * (1 - deltaY) + b.r * deltaX * (1 - deltaY) + c.r * (1 - deltaX) * deltaY + d.r * deltaX * deltaY;

      //blue = a.b;
      //green = a.g;
      //red = a.r;
      
      setPixelRGB(iX, iY, a, nWidth);




   /*   image[offset] = static_cast<unsigned char>(red);
      image[offset + 1] = static_cast<unsigned char>(green);
      image[offset + 2] = static_cast<unsigned char>(blue);
      image[offset + 3] = static_cast<unsigned char>(255);
      offset += 4;
      // skip alpha byte*/
    }
    width = nWidth;
    height = nHeight;
  }

  // 3
}
