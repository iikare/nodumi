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
    defaultScale = scale;
  } 

  position = {0,0};


  imgTex = LoadTextureFromImage(img);
  SetTextureFilter(imgTex, TEXTURE_FILTER_BILINEAR);
  
  isLoaded = true;
}
void imageController::unload() {
 if (isLoaded) {
    UnloadImage(img);
    UnloadTexture(imgTex);
    
    position = {0, 0};
    offset = {0, 0};
    base = {0, 0};
    scale = 1;
    defaultScale = 1;

    isLoaded = false;
  }
}

void imageController::draw() {
  if (isLoaded) {
    DrawTextureEx(imgTex, {(float)position.x + (float)offset.x, (float)position.y + (float)offset.y}, 0, scale, WHITE);
  }
}

void imageController::updatePosition() {
  if (canMove) {
    float xBounds = min(max(0, (int)GetMousePosition().x), ctr.getWidth());
    float yBounds = min(max(ctr.topHeight, (int)GetMousePosition().y), ctr.getHeight()); 

    offset.x = xBounds - base.x;
    offset.y = yBounds - base.y;

    //offset.x = min(max(0, (int)GetMousePosition().x - base.x), ctr.getWidth());
    //offset.y = min(max(ctr.barHeight, (int)GetMousePosition().y - base.y), ctr.getHeight());
  }
}


void imageController::updateBasePosition() {
  canMove = true;
  
  base.x = GetMousePosition().x;
  base.y = GetMousePosition().y;
}

void imageController::finalizePosition() {
  if (canMove) {
    canMove = false;
    position.x += offset.x;
    position.y += offset.y;
    offset = {0, 0};
  }

}

void imageController::changeScale(float scaleOffset) { 
  scale = min(max(0.1f*defaultScale, (float)scale + scaleOffset* scale/defaultScale), 10.0f*defaultScale);
  logQ("newscale", scale);
}
