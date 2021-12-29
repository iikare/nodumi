#include <algorithm>
#include "define.h"
#include "image.h"
#include "enum.h"
#include "log.h"

using std::min;
using std::max;

void imageController::load(string path) {
  //logW(LL_INFO, "load image:", path);

  unload();

  img = LoadImage(path.c_str());

  // find default scale
  if (img.width > ctr.getWidth() || img.height > ctr.getHeight()) {
    scale = 1.0/max((float)img.width/ctr.getWidth(), (float)img.height/ctr.getHeight());
  } 

  position = {0,0};
  rotation = 0;


  imgTex = LoadTextureFromImage(img);
  
  isLoaded = true;
}
void imageController::unload() {

  UnloadImage(img);
  UnloadTexture(imgTex);

  isLoaded = false;
}

void imageController::draw() {
  if (isLoaded) {
    DrawTextureEx(imgTex, {(float)position.x, (float)position.y}, rotation, scale, WHITE);
  }
}
