#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
  #include "../dpd/raylib/src/rlgl.h"
#else
  #include <raylib.h>
  #include <rlgl.h>
#endif

#include <algorithm>
#include <vector>
#include "log.h"
#include "data.h"
#include "color.h"

using std::vector;
using std::min;
using std::max;

class voronoiController {
  public:

    void resample(int voro_y);

    vector<Vector2> vertex;
    vector<colorRGB> color;
  private:


    vector<Vector2> vertex_last;
    vector<colorRGB> color_last;

};
