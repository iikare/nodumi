#pragma once

#include <algorithm>
#include <vector>

#include "build_target.h"
#include "color.h"
#include "data.h"
#include "log.h"

using std::vector;

class voronoiController {
 public:
  void init();

  void unloadData() { unload(); }
  void update();

  void render();

  void resample(int voro_y);

  vector<Vector2> vertex;
  vector<colorRGB> color;

  RenderTexture voro_buffer;
  Texture2D tex;

 private:
  void unload();

  void updateBuffer();
  void updateTexture();

  vector<Vector2> vertex_last;
  vector<colorRGB> color_last;
};
