#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include "../dpd/single-header-image-resampler/base_resample.h"
#include "color.h"
#include "image.h"

using namespace base;
using std::vector;
using std::string;
using std::cerr;
using std::endl;
using std::swap;

BGImage::BGImage() : buffer(0), image(0), oImage(0), dstImage(0), x(0), y(0), width(0), height(0), xOff(0), yOff(0), xOrig(0), yOrig(0), scaleRatio(1), oWidth(0), oHeight(0) {} 

BGImage::~BGImage() {
  delete[] dstImage;
}

void BGImage::loadPNG(string path) {
  cerr << "info: loading PNG - " << path << endl;

  // reset variables
  buffer.clear();
  image.clear();
  x = 0; 
  y = 0;
  width = 0;
  height = 0;
  xOff = 0;
  yOff = 0;
  scaleRatio = 1;

  loadFile(buffer, path);

  int error = decodePNG(image, width, height, buffer.empty() ? 0 : &buffer[0], static_cast<unsigned long>(buffer.size()));
  if (error != 0) {
    cerr << "warn: png decode error: " << error << endl;
  }
  
  oImage = image;
  oWidth = width;
  oHeight = height;

  unsigned char* dstImage = new unsigned char[oWidth * oHeight * 3];
  scale(1, true);

  //cerr << "width, height: " << width << ", " << height << endl;
}

void BGImage::clear() {
  buffer.clear();
  image.clear();
  x = 0;
  y = 0;
  width = 0;
  xOff = 0;
  yOff = 0;
  scaleRatio = 1;
}

colorRGB BGImage::getPixelRGB(int x, int y) {
  // each pixel is 4 bytes
  colorRGB col;
  if (x > static_cast<int>(width) || x < 0 || y > static_cast<int>(height) || y < 0) {
    cerr << "warn: invalid call to getPixelRGB() with {x, y}: {" << x << ", " << y << "}" << endl;
    return col;
  }

  int offset = y * width  * 4 + x * 4;
  
  col.setRGB(image[offset], image[offset + 1], image[offset + 2]);

  return col;
}


void BGImage::scale(double ratio, bool init) {
  if (ratio <= 0 || ratio > 3) {
    cerr << "warn: invalid call to scale() with ratio " << ratio << endl;
    return;
  }
  scaleRatio *= ratio;
  int nWidth = scaleRatio * oWidth;
  int nHeight = scaleRatio * oHeight;
  int offset = 0;

  delete[] dstImage;
  unsigned char* dstImage = new unsigned char[nWidth * nHeight * 3];
  unsigned char* srcImage;
  
  if (!init) {
    unsigned char* srcImage = new unsigned char[oWidth * oHeight * 3];


    for (long unsigned int i = 0; i < oWidth * oHeight * 4; i++) {
      if (i % 4 != 0) {
        srcImage[offset++] = oImage[i - 1];
      }
    }

    image.clear();

    ResampleImage24(srcImage, oWidth, oHeight, dstImage, nWidth, nHeight, KernelTypeLanczos3);
  }

  for (int i = 0; i < nWidth * nHeight * 3; i += 3) {
    image.push_back(dstImage[i]);
    image.push_back(dstImage[i + 1]);
    image.push_back(dstImage[i + 2]);
    image.push_back(0);
  }

  if (!init) {
    delete[] srcImage;
  }

  width = nWidth;
  height = nHeight;
}

void BGImage::flip(bool dir) {
  if (!dir) {
    // flip vertically 
    for (unsigned int j = 0; j <= height/2; j++) {
      for (unsigned int i = 0; i < width; i++) {
        int idx = (j * width + i) * 4;
        int idxMirror = ((height - j) * width + i) * 4;
        swap(image[idx], image[idxMirror]);
        swap(image[idx + 1], image[idxMirror + 1]);
        swap(image[idx + 2], image[idxMirror + 2]);
      }
    }
  }
  else {
    // flip horizontally
    for (unsigned int i = 0; i <= width/2; i++) {
      for (unsigned int j = 0; j < height; j++) {
        int idx = (j * width + i) * 4;
        int idxMirror = (j * width + (width - i)) * 4;
        swap(image[idx], image[idxMirror]);
        swap(image[idx + 1], image[idxMirror + 1]);
        swap(image[idx + 2], image[idxMirror + 2]);
      }
    }
  }
}
