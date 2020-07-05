#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../dpd/single-header-image-resampler/base_resample.h"
#include "color.h"
#include "image.h"
#include "misc.h"

using namespace base;
using std::ifstream;
using std::vector;
using std::string;
using std::cerr;
using std::endl;
using std::swap;

BGImage::BGImage() : buffer(0), image(0), oImage(0),  x(0), y(0), width(0), height(0), xOff(0), yOff(0), xOrig(0), yOrig(0), scaleRatio(1), tint(1), oWidth(0), oHeight(0) {} 

BGImage::~BGImage() {
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

  ifstream filecheck;
  filecheck.open(path);
  if (!filecheck) {
    cerr << "warn: invalid file - " << path << endl;
    return;
  }

  loadFile(buffer, path);

  int error = decodePNG(image, width, height, buffer.empty() ? 0 : &buffer[0], static_cast<unsigned long>(buffer.size()));
  if (error != 0) {
    cerr << "warn: png decode error: " << error << endl;
    return;
  }
  
  oImage = image;
  oWidth = width;
  oHeight = height;

  if (oWidth > WIN_WIDTH && oHeight > WIN_HEIGHT) {
    scale(static_cast<double>(WIN_WIDTH)/oWidth);
  }

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

vector<pixel> BGImage::getKMeansSample() {
  if (!image.size()) {
    cerr << "warn: attempted to get kMeans of non-existent picture" << endl;
    return kMeansData;
  }

  int kWidth = 150;
  int kHeight = 150 * static_cast<double>(height)/width;
  int offset = 0;

  unsigned char* dstImage = new unsigned char[kWidth * kHeight * 3];
  unsigned char* srcImage = new unsigned char[oWidth * oHeight * 3];

  for (int i = 0; i < oWidth * oHeight * 4; i++) {
    if (i % 4 != 0) {
      srcImage[offset++] = oImage[i - 1];
    }
  }

  kMeansData.clear();
  
  // needs to be fast
  ResampleImage24(srcImage, oWidth, oHeight, dstImage, kWidth, kHeight, KernelTypeNearest);

  for (int i = 0; i < kWidth * kHeight * 3; i += 3) {
    colorRGB col(dstImage[i], dstImage[i + 1], dstImage[i + 2]);
    kMeansData.push_back(pixel(col));
  }

  delete[] srcImage;
  delete[] dstImage;

  return kMeansData;
}

void BGImage::scale(double ratio) {
  if (ratio <= 0 || ratio > 3) {
    cerr << "warn: invalid call to scale() with ratio " << ratio << endl;
    return;
  }
  scaleRatio *= ratio;
  int nWidth = scaleRatio * oWidth;
  int nHeight = scaleRatio * oHeight;
  int offset = 0;

  unsigned char* dstImage = new unsigned char[nWidth * nHeight * 3];
  unsigned char* srcImage = new unsigned char[oWidth * oHeight * 3];

  for (int i = 0; i < oWidth * oHeight * 4; i++) {
    if (i % 4 != 0) {
      srcImage[offset++] = oImage[i - 1];
    }
  }

  image.clear();

  ResampleImage24(srcImage, oWidth, oHeight, dstImage, nWidth, nHeight, KernelTypeLanczos3);

  for (int i = 0; i < nWidth * nHeight * 3; i += 3) {
    image.push_back(dstImage[i]);
    image.push_back(dstImage[i + 1]);
    image.push_back(dstImage[i + 2]);
    image.push_back(0);
  }

  delete[] srcImage;
  delete[] dstImage;

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

void BGImage::centerImage(int cX, int cY) {
 x = cX/2 - width/2;
 y = cY/2 - height/2 - MAIN_MENU_HEIGHT/2;
}

void BGImage::changeTint(double amt) {
  if (!amt) {
    // reset tint
    amt = 1.0 / tint;
  }

  tint *= amt;

  for (unsigned int i = 0; i < image.size(); i++) {
    image[i] = min(image[i] * amt, 255.0);
  }
}
