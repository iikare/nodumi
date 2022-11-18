#pragma once

#include "build_target.h"

#include <algorithm>
#include <vector>
#include "log.h"
#include "data.h"
#include "color.h"

using std::vector;

class voronoiController {
  public:

    void unloadData() { unload(); } 
    void updateTexture();

    void resample(int voro_y);


    vector<Vector2> vertex;
    vector<colorRGB> color;

    Texture2D tex;
  private:
    void unload();


    vector<Vector2> vertex_last;
    vector<colorRGB> color_last;

};
