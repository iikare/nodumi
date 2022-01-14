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

  meanV = 0;
  numColors = 0;

  position = {0,0};


  imgTex = LoadTextureFromImage(img);
  SetTextureFilter(imgTex, TEXTURE_FILTER_BILINEAR);
  
  isLoaded = true;

  createRawData();
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

    meanV = 0;
    numColors = 0;

    isLoaded = false;
    rawPixelData.clear();
  }
}

void imageController::draw() {
  if (isLoaded) {
    DrawTextureEx(imgTex, {(float)position.x + (float)offset.x, (float)position.y + (float)offset.y}, 0, scale, WHITE);
    //DrawRectangle((float)position.x + (float)offset.x, (float)position.y + (float)offset.y, 
                  //getWidth(), getHeight(), {100,100,100,100});
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
  //logQ("newscale", scale);
}

vector<kMeansPoint> imageController::getRawData() {
  return rawPixelData;
}

void imageController::createRawData() {
  rawPixelData.clear();

  Image copy = ImageCopy(img);
  constexpr int baseWidth = 100;
  ImageResizeNN(&copy, baseWidth, baseWidth * (float)img.width/img.height);
  
  vector<colorRGB> uniqueColors;

  if (isLoaded) {
    // use original image values to prevent effects from scaling
    for (int x = 0; x < copy.width; ++x) {
      for (int y = 0; y < copy.height; ++y) {
        //logQ(colorRGB(GetImageColor(copy, x, y))); 
        Color tmpColorR = GetImageColor(copy, x, y);
        colorRGB tmpColor = {(double)tmpColorR.r, (double)tmpColorR.g, (double)tmpColorR.b};
        meanV += tmpColor.getHSV().v;
        rawPixelData.push_back(kMeansPoint(tmpColor));
        
        if (uniqueColors.size() < MAX_UNIQUE_COLORS && 
            find(uniqueColors.begin(), uniqueColors.end(), tmpColor) == uniqueColors.end()) {
          uniqueColors.push_back(tmpColor);
        }

      }
    } 
  }
  
  meanV /= (copy.width*copy.height);
  numColors = uniqueColors.size();
  logQ("numC", numColors);

  UnloadImage(copy);

}

float imageController::getMeanValue() {
  return meanV;
}

int imageController::getNumColors() {
  return numColors;
}
