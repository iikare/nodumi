#pragma once

#include <string>
#include <vector>

#include "box.h"
#include "build_target.h"
#include "color.h"
#include "colorgen.h"

using std::ifstream;
using std::max;
using std::min;
using std::string;
using std::stringstream;
using std::vector;

class imageController {
 public:
  imageController()
      : image(),
        imageTex(),
        isLoaded(false),
        canMove(false),
        position({0, 0}),
        base({0, 0}),
        offset({0, 0}),
        scale(1),
        defaultScale(1),
        meanV(0),
        numColors(0),
        rawPixelData(),
        format(IMAGE_NONE),
        buf(){};

  void load(const string& path);
  void unloadData();

  void render();
  void changeScale(double scaleOffset);

  vector<kMeansPoint> getRawData();
  double getMeanValue();
  int getNumColors();

  bool exists() { return isLoaded; }
  bool movable() { return canMove; }

  int getX() { return position.x + offset.x; }
  int getY() { return position.y + offset.y; }
  int getWidth() { return isLoaded ? image.width * scale : -1; }
  int getHeight() { return isLoaded ? image.height * scale : -1; }

  void updatePosition();
  void updateBasePosition();
  void finalizePosition();

  friend class controller;

 private:
  void createRawData();
  void process();
  void load(stringstream& byteData, int byteSize, int fmt);

  Image image;
  Texture2D imageTex;

  bool isLoaded;
  bool canMove;

  // must be floating-point for scale-in-place to work
  Vector2 position;
  point base;
  point offset;
  double scale;
  double defaultScale;
  double meanV;
  int numColors;

  vector<kMeansPoint> rawPixelData;

  int format;
  stringstream buf;
};
