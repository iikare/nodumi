#pragma once

#include "build_target.h"

class shadowController {
 public:
  void init();
  void update();
  void unload();

  RenderTexture buffer;

 private:
};
