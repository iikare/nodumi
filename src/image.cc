#include <vector>
#include <string>
#include <iostream>
#include "color.h"
#include "image.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

BGImage::BGImage() : buffer(0), image(0), x(0), y(0), width(0), height(0) {}

BGImage::~BGImage() {}

void BGImage::loadPNG(string path) {
  cerr << "info: loading PNG - " << path << endl;

  // reset variables
  buffer.clear();
  image.clear();
  x = 0; 
  y = 0;
  width = 0;
  height = 0;

  loadFile(buffer, path);

  int error = decodePNG(image, width, height, buffer.empty() ? 0 : &buffer[0], static_cast<unsigned long>(buffer.size()));
  if (error != 0) {
    cerr << "warn: png decode error: " << error << endl;
  }
  cerr << "width, height: " << width << ", " << height << endl;
  cerr << "RGBA: " << (int)image[0] << ", " << (int)image[1] << ", " << (int)image[2] << ", " << (int)image[3] << endl;

}

colorRGB BGImage::getPixelRGB (int x, int y) {
  // each pixel is 4 bytes
  int offset = (y % height) * width  * 4 + (x % width) * 4;
  colorRGB col(image[offset], image[offset + 1], image[offset + 2]);

  return col;
}
